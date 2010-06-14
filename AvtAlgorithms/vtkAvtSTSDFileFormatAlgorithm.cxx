/*=========================================================================

   Program: ParaView
   Module:    vtkAvtSTSDFileFormatAlgorithm.cxx

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
#include "vtkAvtSTSDFileFormatAlgorithm.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkPointSet.h"
#include "vtkPolyData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkUniformGrid.h"
#include "vtkUnstructuredGrid.h"

#include "vtkCellData.h"
#include "vtkFieldData.h"
#include "vtkPointData.h"

#include "avtSTSDFileFormat.h"
#include "avtDomainNesting.h"
#include "avtDatabaseMetaData.h"
#include "avtVariableCache.h"
#include "avtScalarMetaData.h"
#include "avtVectorMetaData.h"
#include "TimingsManager.h"

#include "limits.h"

vtkStandardNewMacro(vtkAvtSTSDFileFormatAlgorithm);
//-----------------------------------------------------------------------------
vtkAvtSTSDFileFormatAlgorithm::vtkAvtSTSDFileFormatAlgorithm()
{
  this->OutputType = VTK_UNSTRUCTURED_GRID;
}

//-----------------------------------------------------------------------------
vtkAvtSTSDFileFormatAlgorithm::~vtkAvtSTSDFileFormatAlgorithm()
{
}

//-----------------------------------------------------------------------------
int vtkAvtSTSDFileFormatAlgorithm::RequestDataObject(vtkInformation *,
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
  {
  if (!this->InitializeAVTReader())
    {
    return 0;
    }

  int size = this->MetaData->GetNumMeshes();
  if ( size > 1 )
    {
    vtkErrorMacro("STSD Files can only have a single domain");
    return 0;
    }

  //determine if this is an AMR mesh
  const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( 0 );
  switch(meshMetaData.meshType)
    {
    case AVT_CSG_MESH:
      vtkErrorMacro("Currently we do not support AVT_CSG_MESH.");
      break;
    case AVT_AMR_MESH:
      vtkErrorMacro("Unable to create a AMR mesh in a STSD");
      break;
    case AVT_RECTILINEAR_MESH:
      this->OutputType = VTK_RECTILINEAR_GRID;
      break;
    case AVT_CURVILINEAR_MESH:
      this->OutputType = VTK_STRUCTURED_GRID;
      break;
    case AVT_UNSTRUCTURED_MESH:
    case AVT_POINT_MESH:
      this->OutputType = VTK_UNSTRUCTURED_GRID;
      break;
    case AVT_SURFACE_MESH:
    default:
      this->OutputType = VTK_POLY_DATA;
      break;
    }

  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));

  if ( output && output->GetDataObjectType() == this->OutputType )
    {
    return 1;
    }
  else if ( !output || output->GetDataObjectType() != this->OutputType )
    {
    switch( this->OutputType )
      {
      case VTK_RECTILINEAR_GRID:
        output = vtkRectilinearGrid::New();
        break;
      case VTK_STRUCTURED_GRID:
        output = vtkStructuredGrid::New();
        break;
      case VTK_POLY_DATA:
        output = vtkPolyData::New();
        break;
      case VTK_UNSTRUCTURED_GRID:
      default:
        output = vtkUnstructuredGrid::New();
        break;
      }
    this->GetExecutive()->SetOutputData(0, output);
    output->Delete();
    }

  return 1;
  }

//-----------------------------------------------------------------------------
int vtkAvtSTSDFileFormatAlgorithm::RequestData(vtkInformation *request,
        vtkInformationVector **inputVector, vtkInformationVector *outputVector)
  {
  if (!this->InitializeAVTReader())
    {
    return 0;
    }

  //we have to make sure the visit reader populates its cache
  this->AvtFile->ActivateTimestep(); //only 1 time step in ST files

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkDataSet *output = vtkDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!output)
    {
    vtkErrorMacro("Was unable to determine output type");
    return 0;
    }

  const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( 0 );
  vtkstd::string name = meshMetaData.name;
  vtkDataSet *data = this->AvtFile->GetMesh(0, 0, name.c_str() );

  if (!data)
    {
    vtkErrorMacro("Unable to construct a mesh");
    return 0;
    }
  this->AssignProperties( data, name, 0, 0);
  output->ShallowCopy(data);
  data->Delete();

  this->CleanupAVTReader();
  return 1;
}

//-----------------------------------------------------------------------------
void vtkAvtSTSDFileFormatAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


