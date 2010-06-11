/*=========================================================================

   Program: ParaView
   Module:    vtkAvtSTMDFileFormatAlgorithm.h

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

#ifndef _vtkAvtSTMDFileFormatAlgorithm_h
#define _vtkAvtSTMDFileFormatAlgorithm_h

#include "vtkCompositeDataSetAlgorithm.h"
#include "vtkAvtAlgorithmsExport.h"
#include "vtkStdString.h"
#include "avtMeshMetaData.h"


class vtkDataSet;
class vtkDataArray;
class vtkHierarchicalBoxDataSet;
class vtkMultiPieceDataSet;

//BTX
class vtkAvtSTMDFileFormatAlgorithmInternal;
class avtSTMDFileFormat;
class avtDatabaseMetaData;
class avtVariableCache;
//ETX

class AVTALGORITHMS_EXPORT vtkAvtSTMDFileFormatAlgorithm : public vtkCompositeDataSetAlgorithm
{
public:
  static vtkAvtSTMDFileFormatAlgorithm *New();
  vtkTypeMacro(vtkAvtSTMDFileFormatAlgorithm,vtkCompositeDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkAvtSTMDFileFormatAlgorithm();
  ~vtkAvtSTMDFileFormatAlgorithm();

  //the subclasses need to define these methods
  virtual bool InitializeAVTReader();
  virtual void CleanupAVTReader();


  //needed since we have to change the type we output
  virtual int RequestDataObject(vtkInformation *, vtkInformationVector **,
                                vtkInformationVector *);

  // convenience method
  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  virtual int FillOutputPortInformation(int, vtkInformation *info);

//BTX
  int FillAMR( vtkHierarchicalBoxDataSet *amr, const avtMeshMetaData &meshMetaData,
    const int &domain);
  void FillBlock( vtkMultiPieceDataSet *block, const avtMeshMetaData &meshMetaData );
  void AssignProperties( vtkDataSet *data, const vtkStdString &meshName, const int &domain);

  bool ValidAMR( const avtMeshMetaData &meshMetaData );
  void GetDomainRange(const avtMeshMetaData &meshMetaData, int domain[2]);
  //ETX

  bool IsEvenlySpacedDataArray(vtkDataArray *data);

  avtSTMDFileFormat *AvtFile;
  avtDatabaseMetaData *MetaData;
  avtVariableCache *Cache;
  int OutputType;
  //BTX
  vtkAvtSTMDFileFormatAlgorithmInternal *Internal;
  //ETX

private:
  vtkAvtSTMDFileFormatAlgorithm(const vtkAvtSTMDFileFormatAlgorithm&);
  void operator = (const vtkAvtSTMDFileFormatAlgorithm&);
};
#endif
