/*=Plus=header=begin======================================================
Program: Plus
Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
See License.txt for details.
=========================================================Plus=header=end*/ 

#include "CaptureControlWidget.h"
#include "vtkPlusChannel.h"
#include "vtkVirtualDiscCapture.h"
#include "vtksys/SystemTools.hxx"
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

//-----------------------------------------------------------------------------
CaptureControlWidget::CaptureControlWidget(QWidget* aParent)
: QWidget(aParent)
, m_Device(NULL)
{
  ui.setupUi(this);

  connect(ui.startStopButton, SIGNAL(clicked()), this, SLOT(StartStopButtonPressed()) );
  connect(ui.saveButton, SIGNAL(clicked()), this, SLOT(SaveButtonPressed()) );
  connect(ui.saveAsButton, SIGNAL(clicked()), this, SLOT(SaveAsButtonPressed()) );
  connect(ui.clearRecordedFramesButton, SIGNAL(clicked()), this, SLOT(ClearButtonPressed()) );
  connect(ui.samplingRateSlider, SIGNAL(valueChanged(int) ), this, SLOT( SamplingRateChanged(int) ) );
  connect(ui.snapshotButton, SIGNAL(clicked()), this, SLOT(TakeSnapshot()) );

  ui.startStopButton->setText("Start");

  ui.startStopButton->setPaletteBackgroundColor(QColor::fromRgb(255, 255, 255));
}

//-----------------------------------------------------------------------------
CaptureControlWidget::~CaptureControlWidget()
{
}

//-----------------------------------------------------------------------------
PlusStatus CaptureControlWidget::WriteToFile( QString& aFilename )
{
  m_Device->SetFilename(aFilename.toLatin1().constData());

  // Save
  if( m_Device->CloseFile() != PLUS_SUCCESS )
  {
    LOG_ERROR("Saving failed. Unable to close device.");
    return PLUS_FAIL;
  }

  return PLUS_SUCCESS;
}

//-----------------------------------------------------------------------------
double CaptureControlWidget::GetMaximumFrameRate() const
{
  LOG_TRACE("CaptureControlWidget::GetMaximumFrameRate");

  if (m_Device == NULL )
  {
    LOG_ERROR("Unable to reach valid device!");
    return 0.0;
  }

  return m_Device->GetAcquisitionRate();
}

//-----------------------------------------------------------------------------
void CaptureControlWidget::UpdateBasedOnState()
{
  if( m_Device != NULL )
  {
    ui.startStopButton->setEnabled(true);
    ui.channelIdentifierLabel->setText(QString(m_Device->GetDeviceId()));

    ui.saveAsButton->setEnabled( this->CanSave() );
    ui.saveButton->setEnabled( this->CanSave() );
    ui.clearRecordedFramesButton->setEnabled( m_Device->HasUnsavedData() );
    ui.numberOfRecordedFramesValueLabel->setText( QString::number(m_Device->GetTotalFramesRecorded(), 10) );

    if( m_Device->GetEnableCapturing() )
    {
      ui.actualFrameRateValueLabel->setText( QString::number(m_Device->GetActualFrameRate(), 'f', 2) );
      ui.samplingRateSlider->setEnabled(false);
      ui.startStopButton->setText(QString("Stop"));
      ui.startStopButton->setIcon( QPixmap( ":/icons/Resources/icon_Stop.png" ) );
      ui.startStopButton->setEnabled(true);
    }
    else
    {
      ui.startStopButton->setText(QString("Record"));
      ui.startStopButton->setIcon( QPixmap( ":/icons/Resources/icon_Record.png" ) );
      ui.startStopButton->setFocus();
      ui.startStopButton->setEnabled(true);

      ui.actualFrameRateValueLabel->setText( QString::number(0.0, 'f', 2) );
      ui.samplingRateSlider->setEnabled(true);
    }
  }
  else
  {
    ui.startStopButton->setText("Record");
    ui.startStopButton->setIcon( QPixmap( ":/icons/Resources/icon_Record.png" ) );
    ui.startStopButton->setEnabled(false);

    ui.startStopButton->setEnabled(false);
    ui.saveButton->setEnabled(false);
    ui.clearRecordedFramesButton->setEnabled(false);
    ui.channelIdentifierLabel->setText("");
    ui.samplingRateSlider->setEnabled(false);
    ui.actualFrameRateValueLabel->setText( QString::number(0.0, 'f', 2) );
    ui.numberOfRecordedFramesValueLabel->setText( QString::number(0, 10) );
  }
}

//-----------------------------------------------------------------------------
PlusStatus CaptureControlWidget::SaveToMetafile( std::string aOutput )
{
  return PLUS_SUCCESS;
}

//-----------------------------------------------------------------------------
void CaptureControlWidget::StartStopButtonPressed()
{
  if( m_Device != NULL )
  {
    QString text = ui.startStopButton->text();
    if( QString::compare(text, QString("Record")) == 0 )
    {
      m_Device->SetEnableCapturing(true);
    }
    else
    {
      m_Device->SetEnableCapturing(false);
      ui.actualFrameRateValueLabel->setText(QString("0.0"));
    }

    this->UpdateBasedOnState();
  }
}

//-----------------------------------------------------------------------------
void CaptureControlWidget::SetCaptureDevice(vtkVirtualDiscCapture& aDevice)
{
  m_Device = &aDevice;

  this->SamplingRateChanged(10);

  this->UpdateBasedOnState();
}

//-----------------------------------------------------------------------------
void CaptureControlWidget::SaveButtonPressed()
{
  this->SaveFile();
}

//-----------------------------------------------------------------------------
void CaptureControlWidget::SaveAsButtonPressed()
{
  bool isCapturing = m_Device->GetEnableCapturing();

  // Stop recording
  m_Device->SetEnableCapturing(false);

  // Present dialog, get filename
  QFileDialog* dialog = new QFileDialog(this, QString("Select save file"), QString(vtkPlusConfig::GetInstance()->GetOutputDirectory()), QString("All MetaSequence files (*.mha *.mhd)") );
  dialog->setMinimumSize(QSize(640, 480));
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setFileMode(QFileDialog::AnyFile);
  dialog->setViewMode(QFileDialog::Detail);
  dialog->setDefaultSuffix("mha");
  dialog->exec();

  QApplication::processEvents();

  if( dialog->selectedFiles().size() == 0 )
  {
    m_Device->SetEnableCapturing(isCapturing);
    delete dialog;
    return;
  }

  QString fileName = dialog->selectedFiles().first();
  delete dialog;

  std::string message("");
  if( this->WriteToFile(fileName) != PLUS_FAIL )
  {
    message += "Successfully wrote: ";
    message += fileName.toLatin1().constData();
  }
  else
  {
    message += "Failed to write: ";
    message += fileName.toLatin1().constData();
  }

  this->UpdateBasedOnState();
}

//-----------------------------------------------------------------------------
void CaptureControlWidget::SamplingRateChanged( int aValue )
{
  LOG_TRACE("CaptureControlWidget::RecordingFrameRateChanged(" << aValue << ")"); 

  double maxFrameRate = GetMaximumFrameRate();
  int samplingRate = (int)(pow(2.0, ui.samplingRateSlider->maxValue() - aValue));
  double requestedFrameRate(0.0);

  if (samplingRate>0)
  {
    requestedFrameRate = maxFrameRate / (double)samplingRate;
  }
  else
  {
    LOG_WARNING("samplingRate value is invalid");
    requestedFrameRate = maxFrameRate;
  }

  ui.samplingRateSlider->setToolTip(tr("1 / ").append(QString::number((int)samplingRate)));
  ui.requestedFrameRateValueLabel->setText(QString::number(requestedFrameRate, 'f', 2));

  LOG_INFO("Sampling rate changed to " << aValue << " (matching requested frame rate is " << requestedFrameRate << ")");
  this->m_Device->SetRequestedFrameRate(requestedFrameRate);
}

//-----------------------------------------------------------------------------
void CaptureControlWidget::ClearButtonPressed()
{
  this->Clear();
}

//-----------------------------------------------------------------------------
void CaptureControlWidget::SendStatusMessage( const std::string& aMessage )
{
  emit EmitStatusMessage(aMessage);
}

//-----------------------------------------------------------------------------
void CaptureControlWidget::TakeSnapshot()
{
  LOG_TRACE("CaptureControlWidget::TakeSnapshot"); 

  vtkPlusChannel* aChannel = (*m_Device->GetOutputChannelsStart());
  TrackedFrame frame;
  if( aChannel->GetTrackedFrame(&frame) != PLUS_SUCCESS )
  {
    std::string aMessage("Unable to retrieve tracked frame for device: ");
    aMessage += this->m_Device->GetDeviceId();
    this->SendStatusMessage(aMessage);
    LOG_ERROR(aMessage);
    return;
  }

  // Check if there are any valid transforms
  std::vector<PlusTransformName> transformNames;
  frame.GetCustomFrameTransformNameList(transformNames);
  bool validFrame = false;

  if (transformNames.size() == 0)
  {
    validFrame = true;
  }
  else
  {
    for (std::vector<PlusTransformName>::iterator it = transformNames.begin(); it != transformNames.end(); ++it)
    {
      TrackedFrameFieldStatus status = FIELD_INVALID;
      frame.GetCustomFrameTransformStatus(*it, status);

      if ( status == FIELD_OK )
      {
        validFrame = true;
        break;
      }
    }
  }

  if ( !validFrame )
  {
    std::string aMessage("Warning: Snapshot frame for device ");
    aMessage += this->m_Device->GetDeviceId();
    aMessage += " has no transforms. Success.";
    this->SendStatusMessage(aMessage);
    LOG_WARNING(aMessage); 
  }

  vtkTrackedFrameList* list = vtkTrackedFrameList::New();
  list->AddTrackedFrame(&frame);

  vtkMetaImageSequenceIO* writer = vtkMetaImageSequenceIO::New();
  QString fileName = QString("%1/TrackedImageSequence_Snapshot_%2_%3.mha").arg(vtkPlusConfig::GetInstance()->GetOutputDirectory()).arg(m_Device->GetDeviceId()).arg(vtksys::SystemTools::GetCurrentDateTime("%Y%m%d_%H%M%S").c_str());
  writer->SetFileName(fileName.toLatin1().constData());
  writer->SetTrackedFrameList(list);
  if( writer->Write() != PLUS_SUCCESS )
  {
    std::string aMessage("Unable to write frame for device ");
    aMessage += this->m_Device->GetDeviceId();
    this->SendStatusMessage(aMessage);
    LOG_ERROR(aMessage); 
  }

  std::string aMessage("Snapshot taken for device ");
  aMessage += this->m_Device->GetDeviceId();
  aMessage += " to file: ";
  aMessage += fileName;
  this->SendStatusMessage(aMessage);
  LOG_INFO(aMessage); 

  list->Delete();
  writer->Delete();
}

//-----------------------------------------------------------------------------
void CaptureControlWidget::SetEnableCapturing( bool aCapturing )
{
  if( m_Device != NULL )
  {
    this->m_Device->SetEnableCapturing(aCapturing);

    this->UpdateBasedOnState();
  }
}

//-----------------------------------------------------------------------------
void CaptureControlWidget::SaveFile()
{
  LOG_TRACE("CaptureControlWidget::SaveFile"); 

  // Stop recording
  m_Device->SetEnableCapturing(false);

  QString fileName = QString("%1/TrackedImageSequence_%2_%3.mha").arg(vtkPlusConfig::GetInstance()->GetOutputDirectory()).arg(m_Device->GetDeviceId()).arg(vtksys::SystemTools::GetCurrentDateTime("%Y%m%d_%H%M%S").c_str());

  std::string message("");
  if( this->WriteToFile(fileName) != PLUS_FAIL )
  {
    message += "Successfully wrote: ";
    message += fileName.toLatin1().constData();
  }
  else
  {
    message += "Failed to write: ";
    message += fileName.toLatin1().constData();
  }

  this->SendStatusMessage(message);
  this->UpdateBasedOnState();

  LOG_INFO("Captured tracked frame list saved into '" << fileName.toLatin1().constData() << "'");
}

//-----------------------------------------------------------------------------
void CaptureControlWidget::Clear()
{
  m_Device->SetEnableCapturing(false);
  m_Device->Reset();

  this->UpdateBasedOnState();

  std::string aMessage("Successfully cleared data for device: ");
  aMessage += this->m_Device->GetDeviceId();
  this->SendStatusMessage(aMessage);
}

//-----------------------------------------------------------------------------
bool CaptureControlWidget::CanSave() const
{
  return m_Device->HasUnsavedData();
}

//-----------------------------------------------------------------------------
bool CaptureControlWidget::CanRecord() const
{
  return m_Device != NULL;
}