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

#include "cfdglcanvas.h"

#include "cfdtoken.h"

CFDglCanvas::CFDglCanvas(QWidget *parent, Qt::WindowFlags f) : QOpenGLWidget(parent,f)
{
    currentDisplayError = "No Error";
}

CFDglCanvas::~CFDglCanvas()
{
    clearMeshData();

    //myBuffer.destroy();
    //myVertexArray.destroy();
}

void CFDglCanvas::initializeGL()
{
    //Called once to setup rendering context and load resources (esp. shaders)
    /*
    myVertexArray.create();
    if (myVertexArray.isCreated())
        myVertexArray.bind();

    myBuffer.create();
    myBuffer.bind();
    //myBuffer.allocate()//Not sure how much to allocate
    */

    initializeOpenGLFunctions();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void CFDglCanvas::resizeGL(int w, int h)
{
    myWidth = w;
    myHeight = h;
    recomputeProjectionMat(w, h);
}

void CFDglCanvas::recomputeProjectionMat(int w, int h)
{
    if (myState == CFDDisplayState::TEST_BOX)
    {
        projectionMat.setToIdentity();
        projectionMat.ortho(-5.0f,5.0f,-5.0f,5.0f,-1.0f,1.0f);
    }

    if ((myState == CFDDisplayState::MESH) || (myState == CFDDisplayState::FIELD))
    {
        projectionMat.setToIdentity();

        double rawleft = displayBounds.left();
        double rawright = displayBounds.right();
        double rawbottom = displayBounds.bottom();
        double rawtop = displayBounds.top();

        double rawXCenter = ((rawright - rawleft) / 2.0) + rawleft;
        double rawYCenter = ((rawtop - rawbottom) / 2.0) + rawbottom;

        double imgRatio = ((rawright - rawleft)/(rawtop - rawbottom));
        double viewRatio = ((double)w)/((double)h);
        if (imgRatio == viewRatio)
        {
            projectionMat.ortho(rawleft, rawright, rawbottom, rawtop, -1.0f,1.0f);
        }
        else if (imgRatio > viewRatio)
        {
            double newFactor = imgRatio / viewRatio;
            double newBottom = rawYCenter - (newFactor * (rawYCenter - rawbottom));
            double newTop = rawYCenter + (newFactor * (rawtop - rawYCenter));

            projectionMat.ortho(rawleft, rawright, newBottom, newTop, -1.0f,1.0f);
        }
        else
        {
            double newFactor = viewRatio / imgRatio;
            double newLeft = rawXCenter - (newFactor * (rawXCenter - rawleft));
            double newRight = rawXCenter + (newFactor * (rawright - rawXCenter));

            projectionMat.ortho(newLeft, newRight, rawbottom, rawtop, -1.0f,1.0f);
        }
    }
}

void CFDglCanvas::paintGL()
{
    //Called each time something is to be painted
    initializeOpenGLFunctions();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(projectionMat.data());

    glClear(GL_COLOR_BUFFER_BIT);

    if (myState == CFDDisplayState::TEST_BOX)
    {
        glColor3f(0.0, 0.0, 0.0);

        glBegin(GL_QUADS);
        glVertex3f(-1.0f,-1.0f,0);
        glVertex3f(1.0f,-1.0f,0);
        glVertex3f(1.0f,1.0f,0);
        glVertex3f(-1.0f,1.0f,0);
        glEnd();
    }

    if (myState == CFDDisplayState::MESH)
    {
        glColor3f(0.0, 0.0, 0.0);
        glBegin(GL_LINES);

        for (auto faceItr = faceList.cbegin(); faceItr != faceList.cend(); faceItr++)
        {
            QList<int> aFace = (*faceItr);
            bool allZ0 = isAllZ0(aFace);

            if (allZ0)
            {
                glVertex3f(pointList.at(aFace.last()).at(0),
                           pointList.at(aFace.last()).at(1),0.0);
                glVertex3f(pointList.at(aFace.first()).at(0),
                           pointList.at(aFace.first()).at(1),0.0);

                for (int ind = 1; ind < aFace.size(); ind++)
                {
                    glVertex3f(pointList.at(aFace.at(ind - 1)).at(0),
                               pointList.at(aFace.at(ind - 1)).at(1),0.0);
                    glVertex3f(pointList.at(aFace.at(ind)).at(0),
                               pointList.at(aFace.at(ind)).at(1),0.0);
                }
            }
        }
        glEnd();
    }

    if (myState == CFDDisplayState::FIELD)
    {
        int indexVal = -1;
        for (auto faceItr = faceList.cbegin(); faceItr != faceList.cend(); faceItr++)
        {
            indexVal++;
            QList<int> aFace = (*faceItr);
            bool allZ0 = isAllZ0(aFace);

            if (allZ0)
            {
                double rawData = dataList.at(ownerList.at(indexVal)); //TODO: Probably should check bounds

                double dataVal = (rawData - lowDataVal) / dataSpan;

                glBegin(GL_POLYGON);
                double redVal = 1.0;
                double greenVal = 0.0;
                double blueVal = 1.0;

                if (dataVal > 1.0) dataVal = 1.0;
                else if (dataVal < 0.0) dataVal = 0.0;

                if (dataVal > 0.5)
                {
                    blueVal = 0.3 + 0.7 * ((1.0 - dataVal) / 0.5);
                    greenVal = 0.3 + 0.7 * ((1.0 - dataVal) / 0.5);
                }
                else
                {
                    redVal = 0.3 + 0.7 * (dataVal / 0.5);
                    greenVal = 0.3 + 0.7 * (dataVal / 0.5);
                }

                glColor3f(redVal, greenVal, blueVal);

                for (int ind = 0; ind < aFace.size(); ind++)
                {
                    glVertex3f(pointList.at(aFace.at(ind)).at(0),
                               pointList.at(aFace.at(ind)).at(1),0.0);
                }
                glEnd();
            }
        }
    }
}

bool CFDglCanvas::isAllZ0(QList<int> aFace)
{
    for (auto pointIndItr = aFace.cbegin(); pointIndItr != aFace.cend(); pointIndItr++)
    {
        if (pointList.at(*pointIndItr).at(2) > PRECISION)
        {
            return false;
        }
        if (pointList.at(*pointIndItr).at(2) < -PRECISION)
        {
            return false;
        }
    }
    return true;
}

void CFDglCanvas::setDisplayState(CFDDisplayState newState)
{
    myState = newState;
    //recomputeProjectionMat();
    this->update();
}

bool CFDglCanvas::loadMeshData(QByteArray * rawPointFile, QByteArray * rawFaceFile, QByteArray * rawOwnerFile)
{
    clearMeshData();

    CFDtoken * pointRoot = CFDtoken::lexifyString(rawPointFile);
    CFDtoken * faceRoot = CFDtoken::lexifyString(rawFaceFile);
    CFDtoken * ownerRoot = CFDtoken::lexifyString(rawOwnerFile);

    if (!CFDtoken::parseTokenStream(pointRoot) ||
        !CFDtoken::parseTokenStream(faceRoot) ||
        !CFDtoken::parseTokenStream(ownerRoot))
    {
        currentDisplayError = "Unable to read mesh data files";
        delete pointRoot;
        delete faceRoot;
        delete ownerRoot;
        return false;
    }

    CFDtoken * pointElement = pointRoot->getLargestChildArray();
    CFDtoken * faceElement = faceRoot->getLargestChildArray();
    CFDtoken * ownerElement = ownerRoot->getLargestChildArray();

    if ((pointElement == NULL) || (faceElement == NULL) || (ownerElement == NULL))
    {
        currentDisplayError = "Unable to locate mesh data in files";
        delete pointRoot;
        delete faceRoot;
        delete ownerRoot;
        return false;
    }

    //TODO: Add more validity checks before reading each element
    for (auto itr = pointElement->getChildList().cbegin();
         itr != pointElement->getChildList().cend(); itr++)
    {
        if ((*itr)->getChildSize() != 3)
        {
            currentDisplayError = "Point list does not contain points";
            delete pointRoot; delete faceRoot; delete ownerRoot;
            return false;
        }
        QList<double> aPoint;

        for (auto coordItr = (*itr)->getChildList().cbegin();
             coordItr != (*itr)->getChildList().cend(); coordItr++)
        {
            if ((*coordItr)->getType() == CFDtokenType::INT)
            {
                double tmp = (double) (*coordItr)->getIntVal();
                aPoint.append(tmp);
            }
            else if ((*coordItr)->getType() == CFDtokenType::FLOAT)
            {
                aPoint.append((*coordItr)->getFloatVal());
            }
            else
            {
                currentDisplayError = "Point list does not contain numbers";
                delete pointRoot; delete faceRoot; delete ownerRoot;
                return false;
            }
        }

        pointList.append(aPoint);
    }

    for (auto itr = faceElement->getChildList().cbegin();
         itr != faceElement->getChildList().cend(); itr++)
    {
        QList<int> aFace;

        for (auto elementItr = (*itr)->getChildList().cbegin();
             elementItr != (*itr)->getChildList().cend(); elementItr++)
        {
            if ((*elementItr)->getType() == CFDtokenType::INT)
            {
                aFace.append((*elementItr)->getIntVal());
            }
            else
            {
                currentDisplayError = "Face list does not contain ints";
                delete pointRoot; delete faceRoot; delete ownerRoot;
                return false;
            }
        }

        faceList.append(aFace);
    }

    for (auto itr = ownerElement->getChildList().cbegin();
         itr != ownerElement->getChildList().cend(); itr++)
    {
        if ((*itr)->getType() == CFDtokenType::INT)
        {
            ownerList.append((*itr)->getIntVal());
        }
        else
        {
            currentDisplayError = "Owner list does not contain ints";
            delete pointRoot; delete faceRoot; delete ownerRoot;
            return false;
        }
    }

    currentDisplayError = "No Error";

    delete pointRoot;
    delete faceRoot;
    delete ownerRoot;

    displayBounds.setBottom(pointList.at(0).at(1));
    displayBounds.setTop(pointList.at(0).at(1));
    displayBounds.setLeft(pointList.at(0).at(0));
    displayBounds.setRight(pointList.at(0).at(0));

    for (auto itr = pointList.cbegin(); itr != pointList.cend(); itr++)
    {
        double xVal = (*itr).at(0);
        double yVal = (*itr).at(1);

        if (xVal < displayBounds.left()) displayBounds.setLeft(xVal);
        if (xVal > displayBounds.right()) displayBounds.setRight(xVal);
        if (yVal > displayBounds.top()) displayBounds.setTop(yVal);
        if (yVal < displayBounds.bottom()) displayBounds.setBottom(yVal);
    }

    haveValidMeshData = true;

    return true;
}

bool CFDglCanvas::loadFieldData(QByteArray * rawDataFile, QString valueType)
{
    CFDtoken * dataRoot = CFDtoken::lexifyString(rawDataFile);

    if (!CFDtoken::parseTokenStream(dataRoot))
    {
        currentDisplayError = "Unable to read data file";
        delete dataRoot;
        return false;
    }

    CFDtoken * dataElement = dataRoot->getLargestChildArray();

    if (dataElement == NULL)
    {
        currentDisplayError = "Unable to locate data in data file";
        delete dataRoot;
        return false;
    }

    if (valueType == "scalar")
    {
        for (auto itr = dataElement->getChildList().cbegin();
             itr != dataElement->getChildList().cend(); itr++)
        {
            if (((*itr)->getType() == CFDtokenType::FLOAT) ||
                    ((*itr)->getType() == CFDtokenType::INT))
            {
                dataList.append((*itr)->getFloatVal());
            }
            else
            {
                currentDisplayError = "Data list does not contain floats";
                delete dataRoot;
                return false;
            }
        }
    }
    else if (valueType == "magnitude")
    {
        for (auto itr = dataElement->getChildList().cbegin();
             itr != dataElement->getChildList().cend(); itr++)
        {
            if ((*itr)->getType() != CFDtokenType::DATA_ARRAY)
            {
                currentDisplayError = "Data list does not contain float arrays";
                delete dataRoot;
                return false;
            }

            double sum = 0.0;
            for (auto itr2 = (*itr)->getChildList().cbegin();
                 itr2 != (*itr)->getChildList().cend(); itr2++)
            {
                double rawVal = (*itr2)->getFloatVal();
                sum += rawVal * rawVal;
            }
            dataList.append(sqrt(sum));
        }
    }
    else
    {
        currentDisplayError = "Invalid data type";

        delete dataRoot;
        return false;
    }

    QList<double> sortedList = dataList;

    std::sort(sortedList.begin(), sortedList.end());

    if (sortedList.size() < 50)
    {
        lowDataVal = sortedList.at(0);
        highDataVal = sortedList.last();
    }
    else
    {
        lowDataVal = sortedList.at(19);
        highDataVal = sortedList.at(sortedList.size()-19);
    }

    dataSpan = highDataVal - lowDataVal;

    currentDisplayError = "No Error";

    delete dataRoot;
    return true;
}

bool CFDglCanvas::haveMeshData()
{
    return haveValidMeshData;
}

QString CFDglCanvas::getDisplayError()
{
    return currentDisplayError;
}

void CFDglCanvas::clearMeshData()
{
    currentDisplayError = "No Error";

    pointList.clear();
    faceList.clear();
    ownerList.clear();
    haveValidMeshData = false;
}
