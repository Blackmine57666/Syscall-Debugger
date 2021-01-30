#include <iostream>
#include <QMessageBox>
#include <QProcess>
#include <QComboBox>
#include <QTreeView>
#include <QStandardItemModel>

#include "UIs/UI_debugWindow.h"
#include "utils.h"
#include "UIs/debugWindow.h"

using namespace std;

DebugWindow::DebugWindow(){
	UI.setupUi(this);

	// -----

	connect(UI.bRun, &QPushButton::clicked, this, &DebugWindow::bRun);

	connect(UI.bClearCallLogs, &QPushButton::clicked, this, &DebugWindow::clearCallLogs);
	connect(UI.bPlayPauseTable, &QPushButton::clicked, this, &DebugWindow::playPauseTable);
	connect(UI.bProcessSelect, &QPushButton::clicked, this, &DebugWindow::chooseProcess);
}


void DebugWindow::cleanProcess() {


	if (mainProcess != nullptr) {

		killProcesses();

		mainProcess->treeItem = nullptr;
		for (auto *proc : processes) {
			delete proc;
		}

		processes.clear();
	}
}

void DebugWindow::cleanUI() {
	clearCallLogs();

	UI.processTree->topLevelItem(0)->setText(0, "NA");
	for(auto* i : UI.processTree->topLevelItem(0)->takeChildren()) { // TODO jsp si utile (si utile, faire la même pour le tree des syscalls ?)
		delete i;
	}
}

void DebugWindow::changeView(Process& p) {
	dataMutex.lock();
	displayed = &p;
	UI.callLogs->clear();
	for(Syscall* call : p.calls){
		addEntryStart(*call);
		addEntryEnd(*call);
	}
	dataMutex.unlock();
}

void DebugWindow::addEntryStart(Syscall& call) const {
	call.guessName();
	QTreeWidgetItem* item = new QTreeWidgetItem();
	UI.callLogs->insertTopLevelItem(0, item);

	item->setText(0, *call.name);
	// TODO format for arg type
	for(int i=0;i<6;i++){
		item->setText(i+1, QString::fromStdString(to_string(call.entry.args[i])));
	}
	item->setText(7, "?");

}

void DebugWindow::addEntryEnd(Syscall& call) const {
	if(call.exit.is_error==0xF){
		UI.callLogs->topLevelItem(0)->setText(7, "?");
	}else{
		UI.callLogs->topLevelItem(0)->setText(7, QString::fromStdString(to_string(call.exit.rval)));
	}
}


void DebugWindow::setPID(char* pid) const {
	UI.labelPID->setText(QString(pid));
}

void DebugWindow::setState(char s) const {
	switch(s){
		case 0:{
			UI.labelState->setText("NONE");
			break;
		}
		case 1:{
			UI.labelState->setText("RUNNING");
			break;
		}
		case 2:{
			UI.labelState->setText("EXITED");
			break;
		}
		default:{
			UI.labelState->setText("UNKNOWN");
			break;
		}
	}
}

void DebugWindow::stopTracer(){
	if(tracer!=-1){
		runTracer = false;
		waitpid(tracer, nullptr, 0);
	}
}

void DebugWindow::reset(){
	stopTracer();
	cleanUI();
	cleanProcess();
}