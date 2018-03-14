/*********************************************************************************
**
** Copyright (c) 2017 The Regents of the University of California
**
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice, this
** list of conditions and the following disclaimer.
**
** 2. Redistributions in binary form must reproduce the above copyright notice, this
** list of conditions and the following disclaimer in the documentation and/or other
** materials provided with the distribution.
**
** 3. Neither the name of the copyright holder nor the names of its contributors may
** be used to endorse or promote products derived from this software without specific
** prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
** EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
** SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
** BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
** IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
**
***********************************************************************************/

// Contributors:
// Written by Peter Sempolinski, for the Natural Hazard Modeling Laboratory, director: Ahsan Kareem, at Notre Dame

#ifndef CFDCASEINSTANCE_H
#define CFDCASEINSTANCE_H

#include <QObject>

#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>

class FileTreeNode;
class CFDanalysisType;
class RemoteJobData;
class JobListNode;
enum class RequestState;

class CWE_InterfaceDriver;

enum class StageState {UNREADY, UNRUN, RUNNING, FINISHED, FINISHED_PREREQ, LOADING, ERROR, DOWNLOADING, OFFLINE};
//Stages:
//UNRUN: Parameters changeable, RUN button active
//LOADING: Parameters frozen(visible), no buttons
//RUNNING: Parameters frozen(visible), CANCEL button active
//FINISHED: Parameters frozen(visible), RESULTS button active, ROOLBACK button Active
//ERROR: ROLLBACK/RESET only thing available

enum class CaseState {LOADING, INVALID, READY, DEFUNCT, ERROR, OP_INVOKE, RUNNING, DOWNLOAD, OFFLINE};
enum class InternalCaseState {OFFLINE, INVALID, ERROR, DEFUNCT,
                             TYPE_SELECTED, EMPTY_CASE, INIT_DATA_LOAD,
                             MAKING_FOLDER, COPYING_FOLDER, INIT_PARAM_UPLOAD, READY,
                             USER_PARAM_UPLOAD, WAITING_FOLDER_DEL, RE_DATA_LOAD,
                             STARTING_JOB, STOPPING_JOB, RUNNING_JOB_NORECORD, RUNNING_JOB_YESRECORD,
                             FOLDER_CHECK_STOPPED_JOB, DOWNLOAD};

class CFDcaseInstance : public QObject
{
    Q_OBJECT

public:
    CFDcaseInstance(FileTreeNode * newCaseFolder, CWE_InterfaceDriver * mainDriver);
    CFDcaseInstance(CFDanalysisType * caseType, CWE_InterfaceDriver * mainDriver); //For new cases
    CFDcaseInstance(CWE_InterfaceDriver * mainDriver); // For duplications

    bool isDefunct();
    CaseState getCaseState();
    QString getCaseFolder();
    QString getCaseName();

    //Note: For these, it can always answer "I don't know"
    //But that should only happen in the LOADING/ERROR state
    CFDanalysisType * getMyType();
    QMap<QString, QString> getCurrentParams();
    QMap<QString, StageState> getStageStates();

    //Of the following, only one enacted at a time
    void createCase(QString newName, FileTreeNode * containingFolder);
    void duplicateCase(QString newName, FileTreeNode * containingFolder, FileTreeNode * oldCase);
    void changeParameters(QMap<QString, QString> paramList);
    void startStageApp(QString stageID);
    void rollBack(QString stageToDelete);

    void killCaseConnection();

    void downloadCase(QString destLocalFile);
    void stopJob(QString stage);

signals:
    void detachCase();
    void haveNewState(CaseState newState);

private slots:
    void underlyingFilesUpdated();
    void jobListUpdated();
    void fileTaskDone(RequestState invokeStatus);
    void jobInvoked(RequestState invokeStatus, QJsonDocument* jobData);
    void jobKilled(RequestState invokeStatus);
    void recursiveFileOpDone(bool opSuccess, QString message);

    void caseFolderRemoved();

private:
    void emitNewState(InternalCaseState newState);
    void enactDataReload();
    bool caseDataLoaded();
    bool caseDataInvalid();
    void computeCaseType();

    bool stageStatesEqual(QMap<QString, StageState> * list1, QMap<QString, StageState> * list2);
    QMap<QString, StageState> computeStageStates();
    void computeParamList();
    bool allListedJobsHaveDetails(QMap<QString, const RemoteJobData * > jobList);
    QMap<QString, const RemoteJobData *> getRelevantJobs();

    QByteArray produceJSONparams(QMap<QString, QString> paramList);

    //The various state change functions:
    void state_CopyingFolder_taskDone(RequestState invokeStatus);
    void state_FolderCheckStopped_fileChange_taskDone();
    void state_DataLoad_fileChange_jobList();
    void state_InitParam_taskDone(RequestState invokeStatus);
    void state_MakingFolder_taskDone(RequestState invokeStatus);
    void state_Ready_fileChange_jobList();
    void state_RunningNoRecord_jobList();
    void state_RunningYesRecord_jobList();
    void state_StartingJob_jobInvoked(QString jobID);
    void state_StoppingJob_jobKilled();
    void state_UserParamUpload_taskDone(RequestState invokeStatus);
    void state_WaitingFolderDel_taskDone(RequestState invokeStatus);
    void state_Download_recursiveOpDone();

    bool defunct = false;
    QMap<QString, StageState> storedStageStates;
    QMap<QString, QString> storedParamList;
    QMap<QString, QString> prospectiveNewParamList;
    QString runningID;
    QString runningStage;
    const RemoteJobData * runningJobNode = NULL;
    InternalCaseState myState = InternalCaseState::ERROR;

    CWE_InterfaceDriver * theDriver;

    FileTreeNode * caseFolder = NULL;
    CFDanalysisType * myType = NULL;

    QString expectedNewCaseFolder;
    QString downloadDest;

    QString caseParamFileName = ".caseParams";
};

#endif // CFDCASEINSTANCE_H
