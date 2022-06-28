import os
import unittest
import vtk, qt, ctk, slicer
import numpy
import logging
import math
import tensorflow

REGULARIZATION = 1e-2
DROPOUT_RATE = 0.8
LEARNING_RATE = 1e-4
EPOCHS = 2

#
# Feedforward skill assessment
#

#
# Feedforward Parameters Widget
#

class FeedforwardParametersWidget(qt.QFrame):

  def __init__(self, parent = None):
    qt.QFrame.__init__(self)
  
    self.parameterNode = None

    #
    # Parameters area
    #    
    self.parametersLayout = qt.QFormLayout(self)
    self.setLayout(self.parametersLayout)
        
    #
    # Number of hidden layers
    #    
    self.hiddenLayersSpinBox = qt.QSpinBox()
    self.hiddenLayersSpinBox.setRange(1, 10)
    self.hiddenLayersSpinBox.setSingleStep(1)
    self.hiddenLayersSpinBox.setToolTip("Choose the number of hidden layers in the network.")
    self.parametersLayout.addRow("Number of hidden layers ", self.hiddenLayersSpinBox)

    #
    # Number of hidden units
    #    
    self.hiddenUnitsSpinBox = qt.QSpinBox()
    self.hiddenUnitsSpinBox.setRange(1, 256)
    self.hiddenUnitsSpinBox.setSingleStep(1)
    self.hiddenUnitsSpinBox.setToolTip("Choose the number of hidden units for each layer in the network.")
    self.parametersLayout.addRow("Number of hidden units ", self.hiddenUnitsSpinBox)

    # connections
    self.hiddenLayersSpinBox.connect('valueChanged(double)', self.hiddenLayersChanged)
    self.hiddenUnitsSpinBox.connect('valueChanged(double)', self.hiddenUnitsChanged)
    
    
  def setParameterNode(self, parameterNode):
    # Replace the old observers
    if (self.parameterNode is not None):
      self.parameterNode.RemoveObserver(self.parameterNodeObserverTag)
      
    self.parameterNode = parameterNode
  
    if self.parameterNode.GetAttribute( "HiddenLayers") is None:
      self.parameterNode.SetAttribute("HiddenLayers", str(1))

    if self.parameterNode.GetAttribute( "HiddenUnits") is None:
      self.parameterNode.SetAttribute("HiddenUnits", str(1))

    if ( self.parameterNode is not None ):
      self.parameterNodeObserverTag = self.parameterNode.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateWidgetFromParameterNode)
    self.updateWidgetFromParameterNode()
    
    
  def getParameterNode(self):
    return self.parameterNode
    
    
  def hiddenLayersChanged(self, value):
    if ( self.parameterNode is None ):
      return
    self.parameterNode.SetAttribute("HiddenLayers", str(value))

  def hiddenUnitsChanged(self, value):
    if ( self.parameterNode is None ):
      return
    self.parameterNode.SetAttribute("HiddenUnits", str(value))
    
    
  def updateWidgetFromParameterNode( self, node = None, eventID = None ):
    if ( self.parameterNode is None ):
      return

    self.hiddenLayersSpinBox.setValue(float(self.parameterNode.GetAttribute("HiddenLayers")))
    

#
# data generator for the records
#
class RecordDataGenerator(tensorflow.keras.utils.Sequence):

    'Generates data for Keras'
    def __init__(self, records, labels, batch_size=1, shuffle=True):
        'Initialization'
        self.batch_size = batch_size
        self.shuffle = shuffle

        # Convert the records to a numpy array
        self.records = numpy.array(records)
        self.labels = numpy.array(labels)
        # shuffle if required
        self.on_epoch_end()

    def __len__(self):
        'Denotes the number of batches per epoch'
        return int(numpy.floor(self.records.shape[0] / self.batch_size))

    def __getitem__(self, index):
        'Generate one batch of data'
        # Generate indexes of the batch
        indexes = self.indexes[index * self.batch_size:(index + 1) * self.batch_size]

        # Generate data
        x = self.records[indexes]
        y = self.labels[indexes]

        return x, y

    def on_epoch_end(self):
        'Updates indexes after each epoch'
        if self.records.size == 0:
            return

        self.indexes = numpy.arange(self.records.shape[0])

        if self.shuffle == True:
            numpy.random.shuffle(self.indexes)


#
# Feedforward Assessment
#

class FeedforwardAssessment():


  def __init__( self ):
    pass


  @staticmethod
  def GetGenericDescription():
    descriptionString = "This assessment method uses a feedforward network on metric values to predict skill score." + "\n\n"
    return descriptionString

    
    
  @staticmethod 
  def ComputeSkill(parameterNode, testRecord, trainingRecords, weights, nameRecord, nameLabels, skillLabels):
    hiddenLayers = int(parameterNode.GetAttribute("HiddenLayers"))
    hiddenUnits = int(parameterNode.GetAttribute("HiddenUnits"))

    # Build the model
    print(skillLabels)
    model = FeedforwardAssessment.BuildModel(len(trainingRecords[0]), hiddenLayers, hiddenUnits)
    
    # Create the data generators
    training_generator = RecordDataGenerator(trainingRecords, skillLabels)
    test_generator = RecordDataGenerator([testRecord], [0])

    # Train the model
    model.fit(x=training_generator, epochs=EPOCHS)

    # Predict for the testRecord
    score = model.predict(x=test_generator)
    score = numpy.squeeze(score).item()

    descriptionString = FeedforwardAssessment.GetGenericDescription()
    
    return score, descriptionString


  @staticmethod
  def BuildModel(inputShape, hiddenLayers, hiddenUnits):
    model = tensorflow.keras.models.Sequential()
    model.add(tensorflow.keras.layers.Dense(inputShape, activation="relu", kernel_regularizer=tensorflow.keras.regularizers.l2(REGULARIZATION)))
    model.add(tensorflow.keras.layers.Dropout(DROPOUT_RATE))
    for layer in range(hiddenLayers):
      model.add(tensorflow.keras.layers.Dense(hiddenUnits, activation="relu", kernel_regularizer=tensorflow.keras.regularizers.l2(REGULARIZATION)))
      model.add(tensorflow.keras.layers.Dropout(DROPOUT_RATE))
    model.add(tensorflow.keras.layers.Dense(1, activation="sigmoid", kernel_regularizer=tensorflow.keras.regularizers.l2(REGULARIZATION)))
    
    optimizer = tensorflow.keras.optimizers.Adam(learning_rate=LEARNING_RATE)
    model.compile(loss="binary_crossentropy", optimizer=optimizer, metrics=["accuracy"])
    return model
      
         
    
  # The critical value for feedback
  @staticmethod
  def GetCriticalValue( parameterNode, skillLabels ):
    # Should be half the range of the labels      
    maxSkill = max( skillLabels )
    minSkill = min( skillLabels )
    criticalValue = minSkill + ( maxSkill - minSkill ) / 2.0

    return criticalValue