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

#include "vtkMultiBlockDataSet.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"

#include "vtkFieldData.h"
#include "vtkPointData.h"
#include "vtkCellData.h"

#include "avtSTMDFileFormat.h"
#include "avtDatabaseMetaData.h"
#include "avtScalarMetaData.h"
#include "avtVectorMetaData.h"

vtkStandardNewMacro(vtkAvtSTMDFileFormatAlgorithm);

//-----------------------------------------------------------------------------
vtkAvtSTMDFileFormatAlgorithm::vtkAvtSTMDFileFormatAlgorithm()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);

  this->AvtFile = NULL;
  this->MetaData = NULL;
}

//-----------------------------------------------------------------------------
vtkAvtSTMDFileFormatAlgorithm::~vtkAvtSTMDFileFormatAlgorithm()
{
  this->CleanupAVTReader();
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
    delete this->AvtFile;
    this->AvtFile = NULL;
    }

  if ( this->MetaData )
    {
    delete this->MetaData;
    this->MetaData = NULL;
    }
}

//-----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  if (!this->InitializeAVTReader())
    {
    return 0;
    }

  //grab image extents etc


  this->CleanupAVTReader();
  return 1;
}


//-----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  if (!this->InitializeAVTReader())
    {
    return 0;
    }

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));


  int size = this->MetaData->GetNumMeshes();
  output->SetNumberOfBlocks( size );
  for ( int i=0; i < size; ++i)
    {
    const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( i );
    int subBlockSize = meshMetaData.numBlocks;
    vtkstd::string name = meshMetaData.name;

    vtkMultiBlockDataSet *child = vtkMultiBlockDataSet::New();
    child->SetNumberOfBlocks( subBlockSize );
    for ( int j=0; j < subBlockSize; ++j )
      {
      vtkDataSet *data = this->AvtFile->GetMesh( j, name.c_str() );
      if ( data )
        {
        //place all the scalar&vector properties onto the data
        this->AssignProperties(data,name,j);
        child->SetBlock(j,data);
        }
      data->Delete();
      }
    output->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(),name.c_str());
    output->SetBlock(i,child);
    child->Delete();
    }

  this->CleanupAVTReader();
  return 1;
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
void vtkAvtSTMDFileFormatAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


