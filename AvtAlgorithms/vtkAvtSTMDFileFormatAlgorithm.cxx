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
#include "vtkDataArraySelection.h"

#include "vtkAMRBox.h"
#include "vtkHierarchicalBoxDataSet.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMultiPieceDataSet.h"
#include "vtkPolyData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkUniformGrid.h"
#include "vtkUnstructuredGrid.h"

#include "vtkCellData.h"
#include "vtkFieldData.h"
#include "vtkPointData.h"

#include "vtkUnstructuredGridRelevantPointsFilter.h"
#include "vtkCleanPolyData.h"

#include "avtSTMDFileFormat.h"
#include "avtDomainNesting.h"
#include "avtDatabaseMetaData.h"
#include "avtVariableCache.h"
#include "avtScalarMetaData.h"
#include "avtVectorMetaData.h"
#include "TimingsManager.h"

#include "limits.h"

vtkStandardNewMacro(vtkAvtSTMDFileFormatAlgorithm);

//-----------------------------------------------------------------------------
vtkAvtSTMDFileFormatAlgorithm::vtkAvtSTMDFileFormatAlgorithm()
{
  this->UpdatePiece = 0;
  this->UpdateNumPieces = 0;
  this->OutputType = VTK_MULTIBLOCK_DATA_SET;
}

//-----------------------------------------------------------------------------
vtkAvtSTMDFileFormatAlgorithm::~vtkAvtSTMDFileFormatAlgorithm()
{
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
int vtkAvtSTMDFileFormatAlgorithm::RequestData(vtkInformation *request,
        vtkInformationVector **inputVector, vtkInformationVector *outputVector)
  {
  if (!this->InitializeAVTReader())
    {
    return 0;
    }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  this->UpdatePiece =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
  this->UpdateNumPieces =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());

  if ( this->OutputType == VTK_HIERARCHICAL_BOX_DATA_SET
    && this->MeshArraySelection
    && this->MeshArraySelection->GetNumberOfArraysEnabled() == 1)
    {
    const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( 0 );
    vtkHierarchicalBoxDataSet *output = vtkHierarchicalBoxDataSet::
      SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    this->FillAMR( output, meshMetaData, 0, 0 );

    return 1;
    }

  else if ( this->OutputType == VTK_MULTIBLOCK_DATA_SET )
    {
    vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::
      SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    int size = this->MetaData->GetNumMeshes();
    if ( this->MeshArraySelection )
      {
      //we don't want NULL blocks to be displayed, so get the
      //actual number of meshes the user wants
      size = this->MeshArraySelection->GetNumberOfArraysEnabled();
      }
    output->SetNumberOfBlocks( size );

    vtkMultiPieceDataSet* tempData = NULL;
    int blockIndex=0;
    for ( int i=0; i < this->MetaData->GetNumMeshes(); ++i)
      {
      const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( i );
      vtkstd::string name = meshMetaData.name;

      switch(meshMetaData.meshType)
        {
        case AVT_CSG_MESH:
          vtkErrorMacro("Currently we do not support AVT_CSG_MESH.");
          break;
        case AVT_AMR_MESH:
        case AVT_RECTILINEAR_MESH:
        case AVT_CURVILINEAR_MESH:
        case AVT_UNSTRUCTURED_MESH:
        case AVT_POINT_MESH:
        case AVT_SURFACE_MESH:
        default:
          tempData = vtkMultiPieceDataSet::New();
          this->FillBlock( tempData, meshMetaData, 0 );
          output->SetBlock(blockIndex,tempData);
          tempData->Delete();
          tempData = NULL;
          break;
        }
      output->GetMetaData(blockIndex)->Set(vtkCompositeDataSet::NAME(),name.c_str());
      ++blockIndex;
      }
    }

  this->CleanupAVTReader();
  return 1;
}

//-----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::FillAMR(
  vtkHierarchicalBoxDataSet *amr, const avtMeshMetaData &meshMetaData,
  const int &timestep, const int &domain)
{
  //we first need to determine if this AMR can be safely converted to a
  //ParaView AMR. What this means is that every dataset needs to have regular spacing
  bool valid  = this->ValidAMR( meshMetaData );
  if ( !valid )
    {
    return 0;
    }

  int domainRange[2];
  this->GetDomainRange( meshMetaData, domainRange );

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
      //only load grids inside the domainRange for this processor
      if ( meshIndex >= domainRange[0] &&
        meshIndex < domainRange[1] )
        {
        //get the rgrid from the VisIt reader
        //so we have the origin/spacing/dims
        CATCH_VISIT_EXCEPTIONS(rgrid,
          vtkRectilinearGrid::SafeDownCast(
          this->AvtFile->GetMesh(timestep, meshIndex, name.c_str())));
        if ( !rgrid )
          {
          //downcast failed or an exception was thrown
          continue;
          }

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

        this->AssignProperties( grid, name, timestep, meshIndex);

        //now create the AMR Box
        vtkAMRBox box(extents);
        amr->SetDataSet(i,j,box,grid);

        grid->Delete();
        }
      ++meshIndex;
      }
    }
  delete[] numDataSets;
  return 1;

}

//-----------------------------------------------------------------------------
void vtkAvtSTMDFileFormatAlgorithm::FillBlock(
  vtkMultiPieceDataSet *block, const avtMeshMetaData &meshMetaData,
  const int &timestep )
{
  vtkstd::string name = meshMetaData.name;

  //set the number of pieces in the block
  block->SetNumberOfPieces( meshMetaData.numBlocks );

  int domainRange[2];
  this->GetDomainRange( meshMetaData, domainRange );

  for ( int i=domainRange[0]; i < domainRange[1]; ++i )
    {
    vtkDataSet *data=NULL;
    CATCH_VISIT_EXCEPTIONS(data,
      this->AvtFile->GetMesh(timestep, i, name.c_str()) );
    if ( data )
      {
      int points = data->GetNumberOfPoints();
      //place all the scalar&vector properties onto the data
      this->AssignProperties(data,name,timestep,i);

      //clean the mesh of all points that are not part of a cell
      if ( meshMetaData.meshType == AVT_UNSTRUCTURED_MESH)
        {
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(data);
        vtkUnstructuredGridRelevantPointsFilter *clean =
            vtkUnstructuredGridRelevantPointsFilter::New();
        clean->SetInput( ugrid );
        clean->Update();
        block->SetPiece(i,clean->GetOutput());
        clean->Delete();
        }
      else if(meshMetaData.meshType == AVT_SURFACE_MESH)
        {
        vtkCleanPolyData *clean = vtkCleanPolyData::New();
        clean->SetInput( data );
        clean->ToleranceIsAbsoluteOn();
        clean->SetAbsoluteTolerance(0.0);
        clean->ConvertStripsToPolysOff();
        clean->ConvertPolysToLinesOff();
        clean->ConvertLinesToPointsOff();
        clean->Update();
        block->SetPiece(i,clean->GetOutput());
        clean->Delete();
        }
      else
        {
        block->SetPiece(i,data);
        }
      data->Delete();
      }
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
    vtkRectilinearGrid *rgrid = NULL;
    CATCH_VISIT_EXCEPTIONS(rgrid, vtkRectilinearGrid::SafeDownCast(
      this->AvtFile->GetMesh(0, i, name.c_str()) ) );
    if ( !rgrid )
      {
      //this is not an AMR that ParaView supports
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
//determine which nodes will be read by this processor
void vtkAvtSTMDFileFormatAlgorithm::GetDomainRange(const avtMeshMetaData &meshMetaData, int domain[2])
{
  int numBlock = meshMetaData.numBlocks;
  domain[0] = 0;
  domain[1] = numBlock;

  //1 == load the whole data
  if ( this->UpdateNumPieces > 1 )
    {
    //determine which domains in this mesh this processor is reponsible for
    float percent = (1.0 / this->UpdateNumPieces) * numBlock;
    domain[0] = percent * this->UpdatePiece;
    domain[1] = (percent * this->UpdatePiece) + percent;
    }
}

//-----------------------------------------------------------------------------
void vtkAvtSTMDFileFormatAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


