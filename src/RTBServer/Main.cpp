#include "RTBServer/Main.hpp"

#include "RTBServer/Cells/LoginCell.hpp"
#include "RTBServer/Cells/MatchmakingCell.hpp"

#include "RTBServer/Repositories/ClientsRepository.hpp"

#include "RTBServer/Simulation/Simulation.hpp"

#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"

RTBServer::ServerApplication::ServerApplication(bool generateAssets) : std::enable_shared_from_this<ServerApplication>() {
	Config = nullptr;
	Scheduler = nullptr;
	ClientsRepository = nullptr;
	Networking = nullptr;

	const std::string mediaFolder = "Media/RollTheBall";

	if (generateAssets) {
		if (!RollTheBall::AssetGenerator(mediaFolder).Generate())
			return;
	}
}

RTBServer::ServerApplication::~ServerApplication() {
}

void RTBServer::ServerApplication::Run() {
	std::shared_ptr<ServerApplication> sharedThis = std::static_pointer_cast<ServerApplication>(shared_from_this());

	Config = std::make_shared<WasabiGame::GameConfig>();
	Config->Set("MatchmakingCellUpdatePeriodS", 1.0f);
	Config->Set("MaxSimulationUpdatesPerSecond", 30.0f);

	Scheduler = std::make_shared<WasabiGame::GameScheduler>();
	ClientsRepository = std::make_shared<RTBServer::ServerClientsRepository>();
	Networking = std::make_shared<RTBServer::ServerNetworking>(sharedThis, Config, Scheduler);

	Networking->Initialize();

	// these cells are special, they dont need initialization
	m_cells.push_back(std::make_shared<LoginCell>(sharedThis));
	m_cells.push_back(std::make_shared<MatchmakingCell>(sharedThis));

	Scheduler->LaunchThread("ServerCellUpdater", [this]() { this->UpdateCellsThread(); });
	Scheduler->LaunchThread("ServerCellInitializer", [this]() { this->InitializeCellsThread(); });
	Scheduler->LaunchThread("ServerCellCleaner", [this]() { this->CleanupCellsThread(); });
	Scheduler->LaunchThread("ServerNetworking", [this]() { this->Networking->Update(1.0f / 60.0f); });

	Timer.Start();
	Scheduler->Run();

	Networking->Destroy();
}

void RTBServer::ServerApplication::UpdateCellsThread() {
	//
	// This is running in a standalone thread
	//
	std::vector<std::shared_ptr<ServerCell>> newDeadCells;
	while (true) {
		{
			std::scoped_lock<std::mutex> lock(m_cellsMutex);
			for (int i = 0; i < m_cells.size(); i++) {
				if (!m_cells[i]->Update()) {
					newDeadCells.push_back(m_cells.at(i));
					m_cells.erase(m_cells.begin() + i);
					i--;
				}
			}
		}

		if (newDeadCells.size() > 0) {
			// some cells died, add them to dead cells queue
			std::scoped_lock<std::mutex> lock(m_deadCellsMutex);
			for (auto it = newDeadCells.begin(); it != newDeadCells.end(); it++) {
				m_deadCells.push_back(*it);
			}
			newDeadCells.clear();
		} else
			std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void RTBServer::ServerApplication::InitializeCellsThread() {
	//
	// This is running in a standalone thread
	//
	while (true) {
		std::shared_ptr<ServerCell> initializedCell = nullptr;
		{
			std::scoped_lock<std::mutex> lock(m_uninitializedCellsMutex);
			if (m_uninitializedCells.size() > 0) {
				m_uninitializedCells[0]->Initialize();
				initializedCell = m_uninitializedCells[0];
				m_uninitializedCells.erase(m_uninitializedCells.begin());
			}
		}

		if (initializedCell) {
			std::scoped_lock<std::mutex> lock(m_cellsMutex);
			m_cells.push_back(initializedCell);
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void RTBServer::ServerApplication::CleanupCellsThread() {
	//
	// This is running in a standalone thread
	//
	while (true) {
		{
			std::scoped_lock<std::mutex> lock(m_deadCellsMutex);
			if (m_deadCells.size() > 0) {
				m_deadCells[0]->Cleanup();
				m_deadCells.erase(m_deadCells.begin());
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void RTBServer::ServerApplication::RegisterCell(std::shared_ptr<ServerCell> cell) {
	std::scoped_lock<std::mutex> lock(m_uninitializedCellsMutex);
	m_uninitializedCells.push_back(cell);
}

std::shared_ptr<RTBServer::ServerCell> RTBServer::ServerApplication::GetLoginCell() const {
	return m_cells[0];
}

std::shared_ptr<RTBServer::ServerCell> RTBServer::ServerApplication::GetMatchmakingCell() const {
	return m_cells[1];
}
