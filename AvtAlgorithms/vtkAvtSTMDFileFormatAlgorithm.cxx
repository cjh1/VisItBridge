/*=========================================================================

   Program: ParaView
   Module:    vtkAvtSTMDFileFormatAlgorithm.cxx

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
#include "vtkAvtSTMDFileFormatAlgorithm.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtkCallbackCommand.h"
#include "vtkDataArraySelection.h"

#include "vtkAMRBox.h"
#include "vtkHierarchicalBoxDataSet.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPolyData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkUniformGrid.h"
#include "vtkUnstructuredGrid.h"

#include "vtkCellData.h"
#include "vtkFieldData.h"
#include "vtkPointData.h"


#include "avtSTMDFileFormat.h"
#include "avtDomainNesting.h"
#include "avtDatabaseMetaData.h"
#include "avtVariableCache.h"
#include "avtScalarMetaData.h"
#include "avtVectorMetaData.h"
#include "TimingsManager.h"

vtkStandardNewMacro(vtkAvtSTMDFileFormatAlgorithm);

//-----------------------------------------------------------------------------
vtkAvtSTMDFileFormatAlgorithm::vtkAvtSTMDFileFormatAlgorithm()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);

  this->AvtFile = NULL;
  this->MetaData = NULL;
  this->Cache = NULL;

  this->OutputType = VTK_MULTIBLOCK_DATA_SET;

  this->PointDataArraySelection = vtkDataArraySelection::New();
  this->CellDataArraySelection = vtkDataArraySelection::New();

  // Setup the selection callback to modify this object when an array
  // selection is changed.
  this->SelectionObserver = vtkCallbackCommand::New();
  this->SelectionObserver->SetCallback(&
    vtkAvtSTMDFileFormatAlgorithm::SelectionModifiedCallback);
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
vtkAvtSTMDFileFormatAlgorithm::~vtkAvtSTMDFileFormatAlgorithm()
{
  this->CleanupAVTReader();

  this->CellDataArraySelection->RemoveObserver(this->SelectionObserver);
  this->PointDataArraySelection->RemoveObserver(this->SelectionObserver);
  this->SelectionObserver->Delete();
  this->CellDataArraySelection->Delete();
  this->PointDataArraySelection->Delete();
}

//-----------------------------------------------------------------------------
bool vtkAvtSTMDFileFormatAlgorithm::InitializeAVTReader()
{
  return false;
}

//-----------------------------------------------------------------------------
void vtkAvtSTMDFileFormatAlgorithm::CleanupAVTReader()
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
int vtkAvtSTMDFileFormatAlgorithm::RequestDataObject(vtkInformation *,
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
  {
  if (!this->InitializeAVTReader())
    {
    return 0;
    }


  int size = this->MetaData->GetNumMeshes();

  //determine if this is an AMR mesh
  this->OutputType = VTK_MULTIBLOCK_DATA_SET;
  const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( 0 );
  if ( size == 1 &&  meshMetaData.meshType == AVT_AMR_MESH)
    {
    //verify the mesh is correct
    if ( this->ValidAMR( meshMetaData ) )
      {
      this->OutputType = VTK_HIERARCHICAL_BOX_DATA_SET;
      }
    }

  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkCompositeDataSet *output = vtkCompositeDataSet::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));

  if ( output && output->GetDataObjectType() == this->OutputType )
    {
    return 1;
    }
  else if ( !output || output->GetDataObjectType() != this->OutputType )
    {
    switch( this->OutputType )
      {
      case VTK_HIERARCHICAL_BOX_DATA_SET:
        output = vtkHierarchicalBoxDataSet::New();
        break;
      case VTK_MULTIBLOCK_DATA_SET:
      default:
        output = vtkMultiBlockDataSet::New();
        break;
      }
    this->GetExecutive()->SetOutputData(0, output);
    output->Delete();
    }

  return 1;
  }

//-----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::RequestInformation(vtkInformation *request,
        vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  if (!this->InitializeAVTReader())
    {
    return 0;
    }

  //grab image extents etc if needed

  return 1;
}


//-----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::RequestData(vtkInformation *request,
        vtkInformationVector **inputVector, vtkInformationVector *outputVector)
  {
  if (!this->InitializeAVTReader())
    {
    return 0;
    }

  //we have to make sure the visit reader populates its cache
  this->AvtFile->ActivateTimestep(); //only 1 time step in ST files

  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  if ( this->OutputType == VTK_HIERARCHICAL_BOX_DATA_SET )
    {
    const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( 0 );
    vtkHierarchicalBoxDataSet *output = vtkHierarchicalBoxDataSet::
      SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    this->FillAMR( output, meshMetaData, 0 );

    return 1;
    }

  else if ( this->OutputType == VTK_MULTIBLOCK_DATA_SET )
    {
    vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::
      SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    int size = this->MetaData->GetNumMeshes();
    output->SetNumberOfBlocks( size );

    vtkMultiBlockDataSet* tempMultiBlock = NULL;
    for ( int i=0; i < size; ++i)
      {
      const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( i );
      vtkstd::string name = meshMetaData.name;

      switch(meshMetaData.meshType)
        {
        case AVT_AMR_MESH:
        case AVT_RECTILINEAR_MESH:
        case AVT_CURVILINEAR_MESH:
        case AVT_UNSTRUCTURED_MESH:
        case AVT_POINT_MESH:
        case AVT_SURFACE_MESH:
        case AVT_CSG_MESH:
        default:
          tempMultiBlock = vtkMultiBlockDataSet::New();
          this->FillMultiBlock( tempMultiBlock, meshMetaData );
          output->SetBlock(i,tempMultiBlock);
          tempMultiBlock->Delete();
          tempMultiBlock = NULL;
          break;
        }
      output->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(),name.c_str());
      }
    }

  this->CleanupAVTReader();
  return 1;
}
//-----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::FillOutputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDataObject");
  return 1;
}

//-----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::FillAMR(
  vtkHierarchicalBoxDataSet *amr, const avtMeshMetaData &meshMetaData,
  const int &domain)
{
  //we first need to determine if this AMR can be safely converted to a
  //ParaView AMR. What this means is that every dataset needs to have regular spacing
  bool valid  = this->ValidAMR( meshMetaData );
  if ( !valid )
    {
    return 0;
    }

  //number of levels in the AMR
  int numGroups = meshMetaData.numGroups;
  amr->SetNumberOfLevels(numGroups);

  //determine the ratio for each level
  void_ref_ptr vr = this->Cache->GetVoidRef(meshMetaData.name.c_str(),
                    AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                    0, -1);
  if (!(*vr))
    {
    vr = this->Cache->GetVoidRef("any_mesh",
          AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
          0, -1);
    }

  if (!(*vr))
    {
    vtkErrorMacro("Unable to find cache for dataset");
    return 0;
    }

  avtDomainNesting *domainNesting = reinterpret_cast<avtDomainNesting*>(*vr);
  for ( int i=1; i < numGroups; ++i) //don't need a ratio for level 0
    {
    intVector ratios = domainNesting->GetRatiosForLevel(i,domain);
    //Visit returns the ratio for each dimension and if it is a multiply or divide
    //Currently we just presume the same ratio for each dimension
    //TODO: verify this logic
    if ( ratios[0] >= 2 )
      {
      amr->SetRefinementRatio(i, ratios[0] );
      }
    }

  //determine the number of grids on each level of the AMR
  intVector gids = meshMetaData.groupIds;
  int *numDataSets = new int[ numGroups ];
  for ( int i=0; i < numGroups; ++i)
    {
    numDataSets[i] = 0; //clear the array
    }
  //count the grids at each level
  for ( int i=0; i < gids.size(); ++i )
    {
    ++numDataSets[gids.at(i)];
    }

  //assign the info the the AMR, and create the uniform grids
  vtkstd::string name = meshMetaData.name;
  vtkRectilinearGrid *rgrid = NULL;
  int meshIndex=0;
  for ( int i=0; i < numGroups; ++i)
    {
    amr->SetNumberOfDataSets(i,numDataSets[i]);
    for (int j=0; j < numDataSets[i]; ++j)
      {
      //get the rgrid from the VisIt reader
      //so we have the origin/spacing/dims
      rgrid = vtkRectilinearGrid::SafeDownCast(
        this->AvtFile->GetMesh(meshIndex, name.c_str()));

      double origin[3];
      origin[0] = rgrid->GetXCoordinates()->GetTuple1(0);
      origin[1] = rgrid->GetYCoordinates()->GetTuple1(0);
      origin[2] = rgrid->GetZCoordinates()->GetTuple1(0);

      double spacing[3];
      spacing[0] = ( rgrid->GetXCoordinates()->GetNumberOfTuples() > 2 ) ?
        fabs( rgrid->GetXCoordinates()->GetTuple1(1) -
        rgrid->GetXCoordinates()->GetTuple1(0)): 1;

      spacing[1] = ( rgrid->GetYCoordinates()->GetNumberOfTuples() > 2 ) ?
        fabs( rgrid->GetYCoordinates()->GetTuple1(1) -
        rgrid->GetYCoordinates()->GetTuple1(0)): 1;

      spacing[2] = ( rgrid->GetZCoordinates()->GetNumberOfTuples() > 2 ) ?
        fabs( rgrid->GetZCoordinates()->GetTuple1(1) -
        rgrid->GetZCoordinates()->GetTuple1(0)): 1;

      //set up the extents for the grid
      int extents[6];
      rgrid->GetExtent( extents );

      int dims[3];
      rgrid->GetDimensions( dims );

      //don't need the rgrid anymoe
      rgrid->Delete();
      rgrid = NULL;

      //create the dataset
      vtkUniformGrid *grid = vtkUniformGrid::New();
      grid->SetOrigin( origin );
      grid->SetSpacing( spacing );
      grid->SetDimensions( dims );

      this->AssignProperties( grid, name, meshIndex );

      //now create the AMR Box
      vtkAMRBox box(extents);
      amr->SetDataSet(i,j,box,grid);

      grid->Delete();
      ++meshIndex;
      }
    }
  delete[] numDataSets;
  return 1;

}

//-----------------------------------------------------------------------------
void vtkAvtSTMDFileFormatAlgorithm::FillMultiBlock(
  vtkMultiBlockDataSet *block, const avtMeshMetaData &meshMetaData )
{
  vtkstd::string name = meshMetaData.name;
  int subBlockSize = meshMetaData.numBlocks;
  block->SetNumberOfBlocks( subBlockSize );
  for ( int i=0; i < subBlockSize; ++i )
    {
    vtkDataSet *data = this->AvtFile->GetMesh( i, name.c_str() );
    if ( data )
      {
      //place all the scalar&vector properties onto the data
      this->AssignProperties(data,name,i);
      block->SetBlock(i,data);
      }
    data->Delete();
    }
}

//-----------------------------------------------------------------------------
void vtkAvtSTMDFileFormatAlgorithm::AssignProperties( vtkDataSet *data,
    const vtkStdString &meshName, const int &domain)
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
    vtkDataArray *scalar = this->AvtFile->GetVar(domain,name.c_str());
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
    vtkDataArray *vector = this->AvtFile->GetVectorVar(domain,name.c_str());
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
bool vtkAvtSTMDFileFormatAlgorithm::ValidAMR( const avtMeshMetaData &meshMetaData )
{

  //I can't find an easy way to determine the type of a sub mesh
  vtkstd::string name = meshMetaData.name;
  vtkRectilinearGrid *rgrid = NULL;

  for ( int i=0; i < meshMetaData.numBlocks; ++i )
    {
    //lets get the mesh for each amr box
    rgrid = vtkRectilinearGrid::SafeDownCast(
      this->AvtFile->GetMesh(i,name.c_str() )  );
    if ( !rgrid )
      {
      //this is not an AMR that ParaView supports
      rgrid->Delete();
      return false;
      }

    //verify the spacing of the grid is uniform
    if (!this->IsEvenlySpacedDataArray( rgrid->GetXCoordinates()) )
      {
      rgrid->Delete();
      return false;
      }
    if (!this->IsEvenlySpacedDataArray( rgrid->GetYCoordinates()) )
      {
      rgrid->Delete();
      return false;
      }
    if (!this->IsEvenlySpacedDataArray( rgrid->GetZCoordinates()) )
      {
      rgrid->Delete();
      return false;
      }
    rgrid->Delete();
    }

  return true;
}
//-----------------------------------------------------------------------------
bool vtkAvtSTMDFileFormatAlgorithm::IsEvenlySpacedDataArray(vtkDataArray *data)
{
  if ( !data )
    {
    return false;
    }

  //if we have less than 3 values it is evenly spaced
  vtkIdType size = data->GetNumberOfTuples();
  bool valid = true;
  if ( size > 2 )
    {
    double spacing = data->GetTuple1(1)-data->GetTuple1(0);
    double tolerance = 0.000001;
    for (vtkIdType j = 2; j < data->GetNumberOfTuples() && valid; ++j )
      {
      double temp = data->GetTuple1(j) - data->GetTuple1(j-1);
      valid = ( (temp - tolerance) <= spacing ) && ( (temp + tolerance) >= spacing ) ;
      }
    }
  return valid;
}

//----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::GetNumberOfPointArrays()
{
  return this->PointDataArraySelection->GetNumberOfArrays();
}

//----------------------------------------------------------------------------
const char* vtkAvtSTMDFileFormatAlgorithm::GetPointArrayName(int index)
{
  return this->PointDataArraySelection->GetArrayName(index);
}

//----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::GetPointArrayStatus(const char* name)
{
  return this->PointDataArraySelection->ArrayIsEnabled(name);
}

//----------------------------------------------------------------------------
void vtkAvtSTMDFileFormatAlgorithm::SetPointArrayStatus(const char* name, int status)
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
int vtkAvtSTMDFileFormatAlgorithm::GetNumberOfCellArrays()
{
  return this->CellDataArraySelection->GetNumberOfArrays();
}

//----------------------------------------------------------------------------
const char* vtkAvtSTMDFileFormatAlgorithm::GetCellArrayName(int index)
{
  return this->CellDataArraySelection->GetArrayName(index);
}

//----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::GetCellArrayStatus(const char* name)
{
  return this->CellDataArraySelection->ArrayIsEnabled(name);
}

//----------------------------------------------------------------------------
void vtkAvtSTMDFileFormatAlgorithm::SetCellArrayStatus(const char* name, int status)
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
void vtkAvtSTMDFileFormatAlgorithm::SelectionModifiedCallback(vtkObject*, unsigned long,
                                             void* clientdata, void*)
{
  static_cast<vtkAvtSTMDFileFormatAlgorithm*>(clientdata)->Modified();
}

//-----------------------------------------------------------------------------
void vtkAvtSTMDFileFormatAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


