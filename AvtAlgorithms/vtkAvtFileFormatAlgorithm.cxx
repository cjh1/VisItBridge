/*=========================================================================

   Program: ParaView
   Module:    vtkAvtFileFormatAlgorithm.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "vtkAvtFileFormatAlgorithm.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtkCallbackCommand.h"
#include "vtkDataArraySelection.h"

#include "vtkDataSet.h"

#include "vtkCellData.h"
#include "vtkFieldData.h"
#include "vtkPointData.h"

#include "avtFileFormat.h"
#include "avtDomainNesting.h"
#include "avtDatabaseMetaData.h"
#include "avtVariableCache.h"
#include "avtScalarMetaData.h"
#include "avtVectorMetaData.h"
#include "TimingsManager.h"

#include "limits.h"

vtkStandardNewMacro(vtkAvtFileFormatAlgorithm);

//-----------------------------------------------------------------------------
vtkAvtFileFormatAlgorithm::vtkAvtFileFormatAlgorithm()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);

  this->AvtFile = NULL;
  this->MetaData = NULL;
  this->Cache = NULL;

  this->PointDataArraySelection = vtkDataArraySelection::New();
  this->CellDataArraySelection = vtkDataArraySelection::New();

  // Setup the selection callback to modify this object when an array
  // selection is changed.
  this->SelectionObserver = vtkCallbackCommand::New();
  this->SelectionObserver->SetCallback(&
    vtkAvtFileFormatAlgorithm::SelectionModifiedCallback);
  this->SelectionObserver->SetClientData(this);
  this->PointDataArraySelection->AddObserver(vtkCommand::ModifiedEvent,
                                             this->SelectionObserver);
  this->CellDataArraySelection->AddObserver(vtkCommand::ModifiedEvent,
                                            this->SelectionObserver);

  //visit has this horrible singelton timer that is called in all algorithms
  //we need to initiailize it, and than disable it
  if ( !visitTimer )
    {
    TimingsManager::Initialize("");
    visitTimer->Disable();
    }
}

//-----------------------------------------------------------------------------
vtkAvtFileFormatAlgorithm::~vtkAvtFileFormatAlgorithm()
{
  this->CleanupAVTReader();

  this->CellDataArraySelection->RemoveObserver(this->SelectionObserver);
  this->PointDataArraySelection->RemoveObserver(this->SelectionObserver);
  this->SelectionObserver->Delete();
  this->CellDataArraySelection->Delete();
  this->PointDataArraySelection->Delete();
}

//-----------------------------------------------------------------------------
bool vtkAvtFileFormatAlgorithm::InitializeAVTReader()
{
  return false;
}

//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::CleanupAVTReader()
{
  if ( this->AvtFile )
    {
    this->AvtFile->FreeUpResources();
    delete this->AvtFile;
    this->AvtFile = NULL;
    }

  if ( this->MetaData )
    {
    delete this->MetaData;
    this->MetaData = NULL;
    }

  if ( this->Cache )
    {
    delete this->Cache;
    this->Cache = NULL;
    }
}


//-----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::RequestInformation(vtkInformation *request,
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  if (!this->InitializeAVTReader())
    {
    return 0;
    }
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  if ( this->MetaData->GetNumMeshes() > 0 )
    {
    int maxPieces = (this->MetaData->GetMeshes(0).numBlocks > 1)?
      -1:1;
    //only MD classes have blocks inside a mesh, and therefore
    //we can use that to determine if we support reading on each processor
    outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(),
      maxPieces);
    }

  //Set up ghost levels

  //setup user selection of arrays to load
  this->SetupDataArraySelections();

  //setup the timestep and cylce info
  this->SetupTemporalInformation(outInfo);


  return 1;
}


//-----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::RequestData(vtkInformation *request,
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  return 1;
}

//-----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::RequestUpdateExtent(vtkInformation *request,
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  return 1;
}

//-----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::FillOutputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDataObject");
  return 1;
}


//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::AssignProperties( vtkDataSet *data,
    const vtkStdString &meshName, const int &timestep, const int &domain)
{
  int size = this->MetaData->GetNumScalars();
  for ( int i=0; i < size; ++i)
    {
    const avtScalarMetaData scalarMeta = this->MetaData->GetScalars(i);
    if ( meshName != scalarMeta.meshName )
      {
      //this mesh doesn't have this scalar property, go to next
      continue;
      }

    vtkstd::string name = scalarMeta.name;

    //now check agianst what arrays the user has selected to load
    bool selected = false;
    if (scalarMeta.centering == AVT_ZONECENT)
      {
      //cell array
      selected = this->GetCellArrayStatus(name.c_str());
      }
    else if (scalarMeta.centering == AVT_NODECENT)
      {
      //point array
      selected = this->GetPointArrayStatus(name.c_str());
      }
    if (!selected)
      {
      //don't add the array since the user hasn't selected it
      continue;
      }

    vtkDataArray *scalar = this->AvtFile->GetVar(timestep,domain,name.c_str());
    if ( !scalar )
      {
      //it seems that we had a bad array for this domain
      continue;
      }

    //update the vtkDataArray to have the name, since GetVar doesn't require
    //placing a name on the returned array
    scalar->SetName( name.c_str() );

    //based on the centering we go determine if this is cell or point based
    switch(scalarMeta.centering)
      {
      case AVT_ZONECENT:
        //cell property
        data->GetCellData()->AddArray( scalar );
        break;
      case AVT_NODECENT:
        //point based
        data->GetPointData()->AddArray( scalar );
        break;
      case AVT_NO_VARIABLE:
      case AVT_UNKNOWN_CENT:
      default:
        break;
      }
    scalar->Delete();
    }

  //now do vector properties
  size = this->MetaData->GetNumVectors();
  for ( int i=0; i < size; ++i)
    {
    const avtVectorMetaData vectorMeta = this->MetaData->GetVectors(i);
    if ( meshName != vectorMeta.meshName )
      {
      //this mesh doesn't have this vector property, go to next
      continue;
      }
    vtkstd::string name = vectorMeta.name;

    //now check agianst what arrays the user has selected to load
    bool selected = false;
    if (vectorMeta.centering == AVT_ZONECENT)
      {
      //cell array
      selected = this->GetCellArrayStatus(name.c_str());
      }
    else if (vectorMeta.centering == AVT_NODECENT)
      {
      //point array
      selected = this->GetPointArrayStatus(name.c_str());
      }
    if (!selected)
      {
      //don't add the array since the user hasn't selected it
      continue;
      }

    vtkDataArray *vector = this->AvtFile->GetVectorVar(timestep,domain,name.c_str());
    if ( !vector )
      {
      //it seems that we had a bad array for this domain
      continue;
      }

    //update the vtkDataArray to have the name, since GetVar doesn't require
    //placing a name on the returned array
    vector->SetName( name.c_str() );

    //based on the centering we go determine if this is cell or point based
    switch(vectorMeta.centering)
      {
      case AVT_ZONECENT:
        //cell property
        data->GetCellData()->AddArray( vector );
        break;
      case AVT_NODECENT:
        //point based
        data->GetPointData()->AddArray( vector );
        break;
      case AVT_NO_VARIABLE:
      case AVT_UNKNOWN_CENT:
        break;
      }
    vector->Delete();
    }
}





//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SetupTemporalInformation(
  vtkInformation *outInfo)
{

  int numTimeValues;
  double timeRange[2];
  vtkstd::vector< double > timesteps;
  vtkstd::vector< int > cycles;

  this->AvtFile->FormatGetTimes( timesteps );
  this->AvtFile->FormatGetCycles( cycles );

  bool hasTime = timesteps.size() > 0;
  bool hasCycles = cycles.size() > 0;
  bool hasTimeAndCycles = hasTime && hasCycles;

  //need to figure out the use case of when cycles and timesteps don't match
  if (hasTimeAndCycles && timesteps.size()==cycles.size() )
    {
    //presume that timesteps and cycles are just duplicates of each other
    numTimeValues = static_cast<int>(timesteps.size());
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
      &timesteps[0],numTimeValues);
    timeRange[0] = timesteps[0];
    timeRange[1] = timesteps[numTimeValues-1];
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
      timeRange, 2);
    }
  else if( hasTime )
    {
    numTimeValues = static_cast<int>(timesteps.size());

    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
      &timesteps[0],numTimeValues);
    timeRange[0] = timesteps[0];
    timeRange[1] = timesteps[numTimeValues-1];
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
      timeRange, 2);
    }
  else if( hasCycles )
    {
    //convert the cycles over to time steps now
    for ( unsigned int i=0; i < cycles.size(); ++i)
      {
      timesteps.push_back( static_cast<double>(cycles[i]) );
      }

    numTimeValues = static_cast<int>(timesteps.size());

    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
      &timesteps[0],numTimeValues);
    timeRange[0] = timesteps[0];
    timeRange[1] = timesteps[numTimeValues-1];
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
      timeRange, 2);
    }
}

//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SetupDataArraySelections( )
{
  if (!this->MetaData)
    {
    return;
    }
  //go through the meta data and get all the scalar and vector property names
  //add them to the point & cell selection arrays for user control if they don't already exist
  int size = this->MetaData->GetNumScalars();
  vtkstd::string name;
  for ( int i=0; i < size; ++i)
    {
    const avtScalarMetaData scalarMetaData = this->MetaData->GetScalars(i);
    name = scalarMetaData.name;
    switch(scalarMetaData.centering)
      {
      case AVT_ZONECENT:
        //cell property
        if (!this->CellDataArraySelection->ArrayExists(name.c_str()))
          {
          this->CellDataArraySelection->EnableArray(name.c_str());
          }
        break;
      case AVT_NODECENT:
        //point based
        if (!this->PointDataArraySelection->ArrayExists(name.c_str()))
          {
          this->PointDataArraySelection->EnableArray(name.c_str());
          }
        break;
      case AVT_NO_VARIABLE:
      case AVT_UNKNOWN_CENT:
        break;
      }

    }

  size = this->MetaData->GetNumVectors();
  for ( int i=0; i < size; ++i)
    {
    const avtVectorMetaData vectorMetaData = this->MetaData->GetVectors(i);
    name = vectorMetaData.name;
    switch(vectorMetaData.centering)
      {
      case AVT_ZONECENT:
        //cell property
        if (!this->CellDataArraySelection->ArrayExists(name.c_str()))
          {
          this->CellDataArraySelection->EnableArray(name.c_str());
          }
        break;
      case AVT_NODECENT:
        //point based
        if (!this->PointDataArraySelection->ArrayExists(name.c_str()))
          {
          this->PointDataArraySelection->EnableArray(name.c_str());
          }
        break;
      case AVT_NO_VARIABLE:
      case AVT_UNKNOWN_CENT:
        break;
      }
    }
}
//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::GetNumberOfPointArrays()
{
  return this->PointDataArraySelection->GetNumberOfArrays();
}

//----------------------------------------------------------------------------
const char* vtkAvtFileFormatAlgorithm::GetPointArrayName(int index)
{
  return this->PointDataArraySelection->GetArrayName(index);
}

//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::GetPointArrayStatus(const char* name)
{
  return this->PointDataArraySelection->ArrayIsEnabled(name);
}

//----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SetPointArrayStatus(const char* name, int status)
{
  if(status)
    {
    this->PointDataArraySelection->EnableArray(name);
    }
  else
    {
    this->PointDataArraySelection->DisableArray(name);
    }
}

//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::GetNumberOfCellArrays()
{
  return this->CellDataArraySelection->GetNumberOfArrays();
}

//----------------------------------------------------------------------------
const char* vtkAvtFileFormatAlgorithm::GetCellArrayName(int index)
{
  return this->CellDataArraySelection->GetArrayName(index);
}

//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::GetCellArrayStatus(const char* name)
{
  return this->CellDataArraySelection->ArrayIsEnabled(name);
}

//----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SetCellArrayStatus(const char* name, int status)
{
  if(status)
    {
    this->CellDataArraySelection->EnableArray(name);
    }
  else
    {
    this->CellDataArraySelection->DisableArray(name);
    }
}

//----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SelectionModifiedCallback(vtkObject*, unsigned long,
                                             void* clientdata, void*)
{
  static_cast<vtkAvtFileFormatAlgorithm*>(clientdata)->Modified();
}


//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


