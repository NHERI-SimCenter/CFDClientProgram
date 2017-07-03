/*********************************************************************************
**
** Copyright (c) 2017 The University of Notre Dame
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

#ifndef VWTINTERFACEDRIVER_H
#define VWTINTERFACEDRIVER_H

#include <QCoreApplication>

#include <QtGlobal>
#include <QObject>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTreeView>
#include <QStackedWidget>
#include <QWindow>
#include <QThread>

enum class VWTerrorType: unsigned int;
enum class RequestState;

class RemoteDataInterface;

class AuthForm;

class DebugPanelWindow;
class CWE_MainWindow;
class JobOperator;

class VWTinterfaceDriver : public QObject
{
    Q_OBJECT

public:
    explicit VWTinterfaceDriver();
    ~VWTinterfaceDriver();
    void startup(bool useDebugPanel = false);
    void shutdown();

    void closeAuthScreen();

    RemoteDataInterface * getDataConnection();
    JobOperator * getJobHandler();

public slots:
    void getAuthReply(RequestState authReply);
    void getFatalInterfaceError(QString errText);

private slots:
    void subWindowHidden(bool nowVisible);
    void shutdownCallback();

private:
    RemoteDataInterface * theConnector;
    AuthForm * authWindow;
    JobOperator * myJobHandle = NULL;

    DebugPanelWindow * debugWindow = NULL;
    CWE_MainWindow * mainWindow;

    bool doingShutdown = false;
};

#endif // VWTINTERFACEDRIVER_H
