/*=========================================================================

   Program: ParaView
   Module:    vtkVisItFluentReader.cxx

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
#include "vtkVisItFluentReader.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkUnstructuredGrid.h"

#include "avtFluentFileFormat.h"
#include "avtDatabaseMetaData.h"
#include "avtMeshMetaData.h"

vtkStandardNewMacro(vtkVisItFluentReader);

//-----------------------------------------------------------------------------
vtkVisItFluentReader::vtkVisItFluentReader()
{
  this->FileName = 0;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);

  this->AvtReader = NULL;
  this->ReaderMetaData = NULL;
}

//-----------------------------------------------------------------------------
vtkVisItFluentReader::~vtkVisItFluentReader()
{
  this->SetFileName(0);
  if ( this->AvtReader )
    {
    delete this->AvtReader;
    }

  if ( this->ReaderMetaData )
    {
    delete this->ReaderMetaData;
    }
}
//-----------------------------------------------------------------------------
int vtkVisItFluentReader::CanReadFile(const char *fname)
{
  if ( this->AvtReader )
    {
    return true;
    }

  int ret = 0;
  try
    {
    this->AvtReader = new avtFluentFileFormat(fname);
    ret = 1;
    }
  catch(...)
    {
    //we are going to catch all exceptions currently
    ret = 0;
    }

  return ret;
}

//-----------------------------------------------------------------------------
int vtkVisItFluentReader::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  if (!this->AvtReader && !this->CanReadFile( this->FileName ))
    {
    return 0;
    }

  //construct an AVT meta data, that we will allow the avt reader to populate
  this->ReaderMetaData = new avtDatabaseMetaData( );

  //get all the meta data the avt reader has
  this->AvtReader->SetDatabaseMetaData( this->ReaderMetaData );

  //now cycle through and convert all the meta data that was added to paraview framework

  return 1;
}


//-----------------------------------------------------------------------------
int vtkVisItFluentReader::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  if (!this->AvtReader )
    {
    return 0;
    }

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  stringVector names = this->ReaderMetaData->GetAllMeshNames();
  size_t size = names.size();
  output->SetNumberOfBlocks( size );

  for ( int i=0; i < size; ++i)
    {
    const avtMeshMetaData *meshMetaData = this->ReaderMetaData->GetMesh( i );
    int subBlockSize = meshMetaData->numBlocks;

    vtkMultiBlockDataSet *child = vtkMultiBlockDataSet::New();
    child->SetNumberOfBlocks( subBlockSize );

    for ( int j=0; j < subBlockSize; ++j )
      {
      vtkUnstructuredGrid *mesh = vtkUnstructuredGrid::SafeDownCast(
      this->AvtReader->GetMesh( j, names.at(i).c_str() ) );
      if ( mesh )
        {
        child->SetBlock(j,mesh);
        }
      mesh->Delete();
      }
    output->SetBlock(i,child);
    child->Delete();
    }
  return 1;
}

//-----------------------------------------------------------------------------
void vtkVisItFluentReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


