/*=========================================================================

   Program: ParaView
   Module:    vtkAvtMTMDFileFormatAlgorithm.cxx

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
#include "vtkAvtMTMDFileFormatAlgorithm.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

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

#include "avtMTSDFileFormat.h"
#include "avtDomainNesting.h"
#include "avtDatabaseMetaData.h"
#include "avtVariableCache.h"
#include "avtScalarMetaData.h"
#include "avtVectorMetaData.h"
#include "TimingsManager.h"

#include "limits.h"

vtkStandardNewMacro(vtkAvtMTMDFileFormatAlgorithm);
//-----------------------------------------------------------------------------
vtkAvtMTMDFileFormatAlgorithm::vtkAvtMTMDFileFormatAlgorithm()
{
  this->OutputType = VTK_MULTIBLOCK_DATA_SET;
}

//-----------------------------------------------------------------------------
vtkAvtMTMDFileFormatAlgorithm::~vtkAvtMTMDFileFormatAlgorithm()
{
}

//-----------------------------------------------------------------------------
int vtkAvtMTMDFileFormatAlgorithm::RequestData(vtkInformation *request,
        vtkInformationVector **inputVector, vtkInformationVector *outputVector)
  {
  if (!this->InitializeAVTReader())
    {
    return 0;
    }
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  int tsLength =
    outInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  double* steps =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

  double TimeStep = 0;

  // Check if a particular time was requested by the pipeline.
  // This overrides the ivar.
  if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS()) && tsLength>0)
    {
    // Get the requested time step. We only supprt requests of a single time
    // step in this reader right now
    double *requestedTimeSteps =
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS());

    // find the first time value larger than requested time value
    // this logic could be improved
    int cnt = 0;
    while (cnt < tsLength-1 && steps[cnt] < requestedTimeSteps[0])
      {
      cnt++;
      }
    TimeStep = steps[cnt];
    }


  //we have to make sure the visit reader populates its cache
  //with the proper timestep
  avtMTSDFileFormat *mtsdFF = static_cast<avtMTSDFileFormat*>(this->AvtFile);
  mtsdFF->ActivateTimestep( TimeStep );


  this->UpdatePiece =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
  this->UpdateNumPieces =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());

  if ( this->OutputType == VTK_HIERARCHICAL_BOX_DATA_SET )
    {
    const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( 0 );
    vtkHierarchicalBoxDataSet *output = vtkHierarchicalBoxDataSet::
      SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    this->FillAMR( output, meshMetaData, TimeStep, 0);
    return 1;
    }

  else if ( this->OutputType == VTK_MULTIBLOCK_DATA_SET )
    {
    vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::
      SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    int size = this->MetaData->GetNumMeshes();
    output->SetNumberOfBlocks( size );

    vtkMultiPieceDataSet* tempData = NULL;
    for ( int i=0; i < size; ++i)
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
          this->FillBlock( tempData, meshMetaData, TimeStep );
          output->SetBlock(i,tempData);
          tempData->Delete();
          tempData = NULL;
          break;
        }
      output->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(),name.c_str());
      }
    }

  this->CleanupAVTReader();
  return 1;
}

//-----------------------------------------------------------------------------
void vtkAvtMTMDFileFormatAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


