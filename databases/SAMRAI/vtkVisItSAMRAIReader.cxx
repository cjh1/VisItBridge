/*=========================================================================

   Program: ParaView
   Module:    vtkVisItSAMRAIReader.cxx

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
#include "vtkVisItSAMRAIReader.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMultiBlockDataSet.h"

#include "avtSAMRAIFileFormat.h"
#include "avtDatabaseMetaData.h"
#include "avtMeshMetaData.h"

vtkStandardNewMacro(vtkVisItSAMRAIReader);

//-----------------------------------------------------------------------------
vtkVisItSAMRAIReader::vtkVisItSAMRAIReader()
{
  this->FileName = 0;
}

//-----------------------------------------------------------------------------
vtkVisItSAMRAIReader::~vtkVisItSAMRAIReader()
{
  this->SetFileName(0);
}
//-----------------------------------------------------------------------------
int vtkVisItSAMRAIReader::CanReadFile(const char *fname)
{
  int ret = 0;
  if ( this->AvtFile )
    {
    delete this->AvtFile;
    }

  try
    {
    this->AvtFile = new avtSAMRAIFileFormat(fname);
    ret = 1;
    }
  catch(...)
    {
    ret = 0;
    }
  return ret;
}

//-----------------------------------------------------------------------------
bool vtkVisItSAMRAIReader::InitializeAVTReader( )
{
  if (!this->AvtFile)
    {
    this->CanReadFile(this->FileName);
    }

  if (!this->MetaData)
    {
    this->MetaData = new avtDatabaseMetaData( );
    }

  //get all the meta data the avt reader has
  this->AvtFile->SetDatabaseMetaData( this->MetaData );

  return 1;
}


//-----------------------------------------------------------------------------
void vtkVisItSAMRAIReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


