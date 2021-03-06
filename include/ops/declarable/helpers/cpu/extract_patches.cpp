//
//  @author sgazeos@gmail.com
//

#include <ops/declarable/helpers/axis.h>
#include <NDArrayFactory.h>

namespace nd4j {
namespace ops {
namespace helpers {

    template <typename T>
    void extractPatches(NDArray<T>* images, NDArray<T>* output, 
        int sizeRow, int sizeCol, int stradeRow, int stradeCol, int rateRow, int rateCol, bool theSame){
        std::vector<int> restDims({1, 2, 3}); // the first and the last dims
        std::unique_ptr<ResultSet<T>> listOfMatricies(NDArrayFactory<T>::allTensorsAlongDimension(images, restDims));
        std::unique_ptr<ResultSet<T>> listOfOutputs(NDArrayFactory<T>::allTensorsAlongDimension(output, restDims));
        // 3D matricies - 2D matricies of vectors (if last dim is greater than 1)
        int e = 0;
        int ksizeRowsEffective = sizeRow + (sizeRow - 1) * (rateRow - 1);
        int ksizeColsEffective = sizeCol + (sizeCol - 1) * (rateCol - 1);
        int ksize = ksizeRowsEffective * ksizeColsEffective;
        int batchCount = listOfMatricies->size(); //lengthOf() / ksize;
        int lastDim = images->sizeAt(3);
        int rowDim = images->sizeAt(1);
        int colDim = images->sizeAt(2);

#pragma omp parallel for if(batchCount > Environment::getInstance()->elementwiseThreshold()) schedule(static)
        for (int e = 0; e < batchCount; ++e) {
            NDArray<T>* patch = listOfMatricies->at(e);
            NDArray<T>* outMatrix = listOfOutputs->at(e);
            int startRow = 0;
            int startCol = 0;
            int pos = 0;
            for (int i = 0; i < rowDim; i += stradeRow) 
            for (int j = 0; j < colDim; j += stradeCol) 
                for (int l = 0; l < sizeRow && l + i < rowDim; l++)
                for (int m = 0; m < sizeCol && m + j < colDim; m++) {
                for (int k = 0; k < lastDim; ++k) {
                    (*outMatrix)(pos++) = (*patch)(i + rateRow * l, j + m * rateCol, k);
                    if (pos >= outMatrix->lengthOf()) { k = lastDim; m = sizeCol; l = sizeRow; j = colDim; i = rowDim; }
                }
            }
        }
    }

    template void extractPatches(NDArray<float>* input, NDArray<float>* output, int sizeRow, int sizeCol, int stradeRow, int stradeCol, int rateRow, int rateCol, bool theSame);
    template void extractPatches(NDArray<float16>* input, NDArray<float16>* output, int sizeRow, int sizeCol, int stradeRow, int stradeCol, int rateRow, int rateCol, bool theSame);
    template void extractPatches(NDArray<double>* input, NDArray<double>* output, int sizeRow, int sizeCol, int stradeRow, int stradeCol, int rateRow, int rateCol, bool theSame);
}
}
}