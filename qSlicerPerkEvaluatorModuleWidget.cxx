/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QtCore>
#include <QtGui>

// SlicerQt includes
#include "qSlicerPerkEvaluatorModuleWidget.h"
#include "ui_qSlicerPerkEvaluatorModule.h"

#include "vtkSlicerPerkEvaluatorLogic.h"



//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPerkEvaluatorModuleWidgetPrivate: public Ui_qSlicerPerkEvaluatorModule
{
  Q_DECLARE_PUBLIC( qSlicerPerkEvaluatorModuleWidget );
protected:
  qSlicerPerkEvaluatorModuleWidget* const q_ptr;
public:
  qSlicerPerkEvaluatorModuleWidgetPrivate( qSlicerPerkEvaluatorModuleWidget& object );
  vtkSlicerPerkEvaluatorLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorModuleWidgetPrivate methods


//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorModuleWidgetPrivate
::qSlicerPerkEvaluatorModuleWidgetPrivate( qSlicerPerkEvaluatorModuleWidget& object )
 : q_ptr( &object )
{
}


//-----------------------------------------------------------------------------
vtkSlicerPerkEvaluatorLogic*
qSlicerPerkEvaluatorModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerPerkEvaluatorModuleWidget );
  return vtkSlicerPerkEvaluatorLogic::SafeDownCast( q->logic() );
}





//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorModuleWidget methods


//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorModuleWidget::qSlicerPerkEvaluatorModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerPerkEvaluatorModuleWidgetPrivate( *this ) )
{
}


//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorModuleWidget::~qSlicerPerkEvaluatorModuleWidget()
{
}



void qSlicerPerkEvaluatorModuleWidget
::OnImportClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  QString filename = QFileDialog::getOpenFileName( this, tr("Open record"), "", tr("XML Files (*.xml)") );
  
  if ( filename.isEmpty() == false )
  {
    d->logic()->ImportFile( filename.toStdString() );
  }
  
  d->logic()->SetCurrentTime( d->logic()->GetMinTime() );
  this->UpdateGUI();
}


//-----------------------------------------------------------------------------
void qSlicerPerkEvaluatorModuleWidget::setup()
{
  Q_D(qSlicerPerkEvaluatorModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  connect( d->ImportButton, SIGNAL( pressed() ), this, SLOT( OnImportClicked() ) );
}



void qSlicerPerkEvaluatorModuleWidget
::UpdateGUI()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  d->TotalTimeLabel->setText( QString::number( d->logic()->GetTotalTime(), 'f', 2 ) );
  
  d->PlaybackSlider->setMinimum( 0 );
  d->PlaybackSlider->setMaximum( d->logic()->GetMaxTime() - d->logic()->GetMinTime() );
  
}

