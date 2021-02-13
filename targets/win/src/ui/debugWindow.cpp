#include <iostream>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>

#include "qt/UI_debugWindow.h"
#include "ui/debugWindow.h"
#include "utils.h"

using namespace std;

//template<typename... CArgs>
//void DebugWindow::execFunc(void (*func)(CArgs...), CArgs&&... args) {
//	func(args...);
//}

DebugWindow::DebugWindow(){
	QtUI.setupUi(this);

	tracerConnect = new Tracer(this);
	// -----

	connect(QtUI.bRun, &QPushButton::clicked, this, &DebugWindow::bRun);
	connect(QtUI.bStop, &QPushButton::clicked, this, &DebugWindow::killProcess);
	connect(QtUI.bClearCallLogs, &QPushButton::clicked, this, &DebugWindow::bClearCallLogs);
	connect(QtUI.bPauseTable, &QPushButton::clicked, this, &DebugWindow::bPauseTable);
	connect(QtUI.bProcessSelect, &QPushButton::clicked, this, &DebugWindow::bChooseProcess);

	connect(QtUI.processTree, &QTreeWidget::itemClicked, this, &DebugWindow::treeClick);

	connect(this, &DebugWindow::SIG_AddEntryStart, this, &DebugWindow::addEntryStart);
	connect(this, &DebugWindow::SIG_addEntryEnd, this, &DebugWindow::addEntryEnd);
	connect(this, &DebugWindow::SIG_removeLastEntry, this, &DebugWindow::removeLastEntry);

}


void DebugWindow::killProcess() {
	if (mainProcess != nullptr) {
		tracerConnect->killProcess();
		mainProcess = nullptr;
	}
}

void DebugWindow::cleanUI() {
	bClearCallLogs();

	QtUI.processTree->topLevelItem(0)->setText(0, "NA");
	for(auto* i : QtUI.processTree->topLevelItem(0)->takeChildren()) {
//		delete i;
	}
}

void DebugWindow::changeView(Process& p) {
	displayed = &p;
//	QtUI.callLogs->clear();
	for(Syscall* call : p.calls){
		addEntryStart(call);
		addEntryEnd(call);
	}
}

void DebugWindow::addEntryStart(Syscall* call) {
	call->guessName();
	QtUI.callLogs->insertRow(0);

	QtUI.callLogs->setItem(0, 0, new QTableWidgetItem(*call->name));

	for(int i=0;i<6;i++){
		QtUI.callLogs->setItem(0, i+1, new QTableWidgetItem(QString::number(call->entry.args[i])));
	}
	QtUI.callLogs->setItem(0, 7, new QTableWidgetItem("?"));
}

void DebugWindow::addEntryEnd(Syscall* call) {
	if(call->exit.is_error==0xF){
		QtUI.callLogs->item(0, 7)->setText("?");
	}else{
		QtUI.callLogs->item(0, 7)->setText(QString::number(call->exit.rval));
	}
}

void DebugWindow::removeLastEntry(){
	QtUI.callLogs->removeRow(config::displayLimit - 1);
}

void DebugWindow::setPID(char* pid) {
	QtUI.labelPID->setText(QString(pid));
}

void DebugWindow::setState(char s) {
	switch(s){
		case 0:{
			QtUI.labelState->setText("NONE");
			break;
		}
		case 1:{
			QtUI.labelState->setText("RUNNING");
			break;
		}
		case 2:{
			QtUI.labelState->setText("EXITED");
			break;
		}
		default:{
			QtUI.labelState->setText("UNKNOWN");
			break;
		}
	}
}

void DebugWindow::cleanUp(){
	cleanUI();

	if(!tracerConnect->isLocal){
		for(auto* proc : tracerConnect->processes){
//			delete proc;
		}
	}
}