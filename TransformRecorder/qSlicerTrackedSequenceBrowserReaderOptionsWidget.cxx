/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

/// Qt includes
#include <QFileInfo>

// CTK includes
#include <ctkFlowLayout.h>
#include <ctkUtils.h>

// VTK includes
#include <vtkNew.h>

/// TransformRecorder includes
#include "qSlicerIOOptions_p.h"
#include "qSlicerTrackedSequenceBrowserReaderOptionsWidget.h"
#include "ui_qSlicerTrackedSequenceBrowserReaderOptionsWidget.h"

/// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TransformRecorder
class qSlicerTrackedSequenceBrowserReaderOptionsWidgetPrivate
  : public qSlicerIOOptionsPrivate, public Ui_qSlicerTrackedSequenceBrowserReaderOptionsWidget
{
public:
};

//-----------------------------------------------------------------------------
qSlicerTrackedSequenceBrowserReaderOptionsWidget::qSlicerTrackedSequenceBrowserReaderOptionsWidget(QWidget* parentWidget)
  : qSlicerIOOptionsWidget(new qSlicerTrackedSequenceBrowserReaderOptionsWidgetPrivate, parentWidget)
{
  Q_D( qSlicerTrackedSequenceBrowserReaderOptionsWidget );
  d->setupUi( this );

  connect( d->UseSceneProxyNodesCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( updateProperties() ) );
}

//-----------------------------------------------------------------------------
qSlicerTrackedSequenceBrowserReaderOptionsWidget::~qSlicerTrackedSequenceBrowserReaderOptionsWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTrackedSequenceBrowserReaderOptionsWidget::updateProperties()
{
  Q_D( qSlicerTrackedSequenceBrowserReaderOptionsWidget );

  d->Properties[ "UseSceneProxyNodes" ] = d->UseSceneProxyNodesCheckBox->isChecked();
}