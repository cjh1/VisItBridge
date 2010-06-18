/*=========================================================================

   Program: ParaView
   Module:    vtkAvtMTSDFileFormatAlgorithm.cxx

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
#include "vtkAvtMTSDFileFormatAlgorithm.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPointSet.h"
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

vtkStandardNewMacro(vtkAvtMTSDFileFormatAlgorithm);
//-----------------------------------------------------------------------------
vtkAvtMTSDFileFormatAlgorithm::vtkAvtMTSDFileFormatAlgorithm()
{
  this->OutputType = VTK_MULTIBLOCK_DATA_SET;
}

//-----------------------------------------------------------------------------
vtkAvtMTSDFileFormatAlgorithm::~vtkAvtMTSDFileFormatAlgorithm()
{
}

//-----------------------------------------------------------------------------
int vtkAvtMTSDFileFormatAlgorithm::RequestData(vtkInformation *request,
        vtkInformationVector **inputVector, vtkInformationVector *outputVector)
  {
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  int tsLength =
    outInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  double* steps =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

  double TimeStepValue = 0;
  unsigned int TimeIndex = 0;
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
    while (TimeIndex < tsLength-1 && steps[TimeIndex] < requestedTimeSteps[0])
      {
      TimeIndex++;
      }
    TimeStepValue = steps[TimeIndex];
    }

  if (!this->InitializeAVTReader( TimeIndex ))
    {
    return 0;
    }

  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!output)
    {
    vtkErrorMacro("Was unable to determine output type");
    return 0;
    }

  int size = this->MetaData->GetNumMeshes();
  output->SetNumberOfBlocks( size );

  vtkstd::string name;
  for ( int i=0; i < size; ++i)
    {
    const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( i );
    name = meshMetaData.name;

    vtkDataSet *data = NULL;
    CATCH_VISIT_EXCEPTIONS(data,
      this->AvtFile->GetMesh(TimeIndex, 0, name.c_str()) );
    if ( data )
      {
      this->AssignProperties( data, name, TimeIndex, 0);

       //clean the mesh of all points that are not part of a cell
      if ( meshMetaData.meshType == AVT_UNSTRUCTURED_MESH)
        {
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(data);
        vtkUnstructuredGridRelevantPointsFilter *clean =
            vtkUnstructuredGridRelevantPointsFilter::New();
        clean->SetInput( ugrid );
        clean->Update();
        output->SetBlock(i,clean->GetOutput());
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
        output->SetBlock(i,clean->GetOutput());
        clean->Delete();
        }
      else
        {
        output->SetBlock(i,data);
        }
      data->Delete();
      output->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(),name.c_str());
      }
    }
  this->CleanupAVTReader();
  return 1;
}

//-----------------------------------------------------------------------------
void vtkAvtMTSDFileFormatAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


