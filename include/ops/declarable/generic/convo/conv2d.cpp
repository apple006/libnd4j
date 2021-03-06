//
// @author Yurii Shyrma, created on 06.03.2018
//

#ifndef LIBND4J_CONVO_OPS_H
#define LIBND4J_CONVO_OPS_H

#include <op_boilerplate.h>
#if NOT_EXCLUDED(OP_conv2d)

#include <op_boilerplate.h>
#include <memory>
#include <ops/declarable/CustomOperations.h>
#include <ops/declarable/OpRegistrator.h>
#include <declarable/generic/helpers/convolutions.h>

namespace nd4j {
    namespace ops {


CUSTOM_OP_IMPL(conv2d, 2, 1, false, 0, 9) {
    
    NDArray<T> *input   = INPUT_VARIABLE(0);                                    // [bS, iH, iW, iC] (NHWC) or [bS, iC, iH, iW] (NCHW)
    NDArray<T> *weights = INPUT_VARIABLE(1);                                    // [kH, kW, iC, oC] (NHWC) or [oC, iC, kH, kW] (NCHW)
    NDArray<T> *bias    = block.width() > 2 ? INPUT_VARIABLE(2) : nullptr;      // [oC]

    NDArray<T> *output  = OUTPUT_VARIABLE(0);                                   // [bS, oH, oW, oC] (NHWC) or [bS, oC, oH, oW] (NCHW)
    
    int kH = INT_ARG(0);                                                        // filter(kernel) height
    int kW = INT_ARG(1);                                                        // filter(kernel) width
    int sH = INT_ARG(2);                                                        // strides height
    int sW = INT_ARG(3);                                                        // strides width
    int pH = INT_ARG(4);                                                        // paddings height
    int pW = INT_ARG(5);                                                        // paddings width
    int dH = INT_ARG(6);                                                        // dilations height
    int dW = INT_ARG(7);                                                        // dilations width
    int isSameMode = INT_ARG(8);                                                // 0-VALID, 1-SAME
    int isNCHW     = block.getIArguments()->size() > 9 ? !INT_ARG(9) : 1;       // 1-NCHW,  0-NHWC

     int bS, iC, iH, iW, oC, oH, oW;                             // batch size, input channels, input height/width, output channels, output height/width;
    int indIOioC, indIiH, indWoC, indWiC, indWkH, indOoH;       // corresponding indexes
    ConvolutionUtils<T>::getSizesAndIndexesConv2d(isNCHW, *input, *output, bS, iC, iH, iW, oC, oH, oW, indIOioC, indIiH, indWiC, indWoC, indWkH, indOoH);

    std::string expectedWeightsShape = ShapeUtils<T>::shapeAsString(ShapeUtils<T>::composeShapeUsingDimsAndIdx({iC,oC,kH,kW,  indWiC,indWoC,indWkH,indWkH+1}));
    REQUIRE_TRUE(expectedWeightsShape == ShapeUtils<T>::shapeAsString(weights), 0, "CUSTOM CONV2D OP: wrong shape of weights array, expected is %s, but got %s instead !", expectedWeightsShape.c_str(), ShapeUtils<T>::shapeAsString(weights).c_str());    
    if (bias) 
        REQUIRE_TRUE(bias->rankOf() <= 2 && oC == bias->lengthOf(), 0, "CUSTOM CONV2D OP: wrong shape of array with biases, expected rank, length: <=2, %i, but got %i, %i instead !", oC, bias->rankOf(), bias->lengthOf());                


    ConvolutionUtils<T>::conv2d({input, weights, bias}, output, {kH,kW,sH,sW,pH,pW,dH,dW,isSameMode,isNCHW});
    
    return Status::OK();
}



DECLARE_SHAPE_FN(conv2d) {

    auto inputShapeInfo   = inputShape->at(0);                                  // [bS, iH, iW, iC] (NHWC) or [bS, iC, iH, iW] (NCHW)
    auto weightsShapeInfo = inputShape->at(1);                                  // [kH, kW, iC, oC] (NHWC) or [oC, iC, kH, kW] (NCHW)
    auto biasShapeInfo    = block.width() > 2 ? inputShape->at(2) : nullptr;    // [oC]

    //output [bS, oH, oW, oC] (NHWC) or [bS, oC, oH, oW] (NCHW)

    int kH = INT_ARG(0);                                                        // filter(kernel) height
    int kW = INT_ARG(1);                                                        // filter(kernel) width
    int sH = INT_ARG(2);                                                        // strides height
    int sW = INT_ARG(3);                                                        // strides width
    int pH = INT_ARG(4);                                                        // paddings height
    int pW = INT_ARG(5);                                                        // paddings width
    int dH = INT_ARG(6);                                                        // dilations height
    int dW = INT_ARG(7);                                                        // dilations width
    int isSameMode = INT_ARG(8);                                                // 0-VALID, 1-SAME
    int isNCHW  = block.getIArguments()->size() > 9 ? !INT_ARG(9) : 1;          // 0-NDHWC, 1-NCDHW

    const int rank = 4;                                                        // 4

    REQUIRE_TRUE(inputShapeInfo[0]   == rank, 0, "CUSTOM CONV2D OP: rank of input array must be equal to %i, but got %i instead !", rank, inputShapeInfo[0]);
    REQUIRE_TRUE(weightsShapeInfo[0] == rank, 0, "CUSTOM CONV2D OP: rank of weights array must be equal to %i, but got %i instead !", rank, weightsShapeInfo[0]);

    int indIOioC, indIiH, indWkH, indWoC, indWiC;
    if(!isNCHW) {
        indIOioC = 3; indIiH = 1; indWkH = 0; indWoC = 3; indWiC = 2;
    }
    else {        
        indIOioC = 1; indIiH = 2; indWkH = 2; indWoC = 0; indWiC = 1;              
    }    

    const int bS = inputShapeInfo[1];                            // batch size
    const int iH = inputShapeInfo[indIiH+1];                     // input height
    const int iW = inputShapeInfo[indIiH+2];                     // input width
    const int iC = inputShapeInfo[indIOioC+1];                   // input channels        
    const int oC = weightsShapeInfo[indWoC+1];                   // output channels

    std::string expectedWeightsShape = ShapeUtils<T>::shapeAsString(ShapeUtils<T>::composeShapeUsingDimsAndIdx({iC,oC,kH,kW,  indWiC,indWoC,indWkH,indWkH+1}));        
    REQUIRE_TRUE(expectedWeightsShape == ShapeUtils<T>::shapeAsString(weightsShapeInfo), 0, "CUSTOM CONV2D OP: wrong shape of weights array, expected is %s, but got %s instead !", expectedWeightsShape.c_str(), ShapeUtils<T>::shapeAsString(weightsShapeInfo).c_str());    
    if (biasShapeInfo) 
        REQUIRE_TRUE(biasShapeInfo[0] <= 2 && oC == shape::length(biasShapeInfo), 0, "CUSTOM CONV2D OP: wrong shape of array with biases, expected rank, length: <=2, %i, but got %i, %i instead !", oC, biasShapeInfo[0], shape::length(biasShapeInfo));
    
    Nd4jLong* outputShapeInfo = nullptr;
    ALLOCATE(outputShapeInfo, block.getWorkspace(), shape::shapeInfoLength(rank), Nd4jLong);

    int oH, oW;                                         // output height, width
    ConvolutionUtils<T>::calcOutSizePool2D(oH, oW, kH, kW, sH, sW, pH, pW, dH, dW, iH, iW, isSameMode);

    outputShapeInfo[0] = rank;
    outputShapeInfo[1] = bS;

    if (isNCHW) {
        outputShapeInfo[2] = oC;
        outputShapeInfo[3] = oH;
        outputShapeInfo[4] = oW;
    } else {
        outputShapeInfo[2] = oH;
        outputShapeInfo[3] = oW;
        outputShapeInfo[4] = oC;
    }
    
    shape::updateStrides(outputShapeInfo, shape::order(inputShapeInfo));
    
    return SHAPELIST(outputShapeInfo);
}



////////////////////////////////////////////////////////////////////////// 
CUSTOM_OP_IMPL(conv2d_bp, 3, 2, false, 0, 9) {
    
    NDArray<T> *input   = INPUT_VARIABLE(0);                                                // [bS, iH, iW, iC] (NHWC) or [bS, iC, iH, iW] (NCHW)
    NDArray<T> *weights = INPUT_VARIABLE(1);                                                // [kH, kW, iC, oC] (NHWC) or [oC, iC, kH, kW] (NCHW)
    NDArray<T> *bias    = block.width() > 3 ? INPUT_VARIABLE(2) : nullptr;                  // [oC]
    NDArray<T> *gradO   = block.width() > 3 ? INPUT_VARIABLE(3) : INPUT_VARIABLE(2);        // [bS, oH, oW, oC] (NHWC) or [bS, oC, oH, oW] (NCHW), epsilon_next
    
    NDArray<T> *gradI = OUTPUT_VARIABLE(0);                                                 // [bS, iH, iW, iC] (NHWC) or [bS, iC, iH, iW] (NCHW), epsilon
    NDArray<T> *gradW = OUTPUT_VARIABLE(1);                                                 // [kH, kW, iC, oC] (NHWC) or [oC, iC, kH, kW] (NCHW)
    NDArray<T> *gradB = block.width() > 3 ? OUTPUT_VARIABLE(2) : nullptr;                   // [oC]
                                     
    int kH = INT_ARG(0);                                                        // filter(kernel) height
    int kW = INT_ARG(1);                                                        // filter(kernel) width
    int sH = INT_ARG(2);                                                        // strides height
    int sW = INT_ARG(3);                                                        // strides width
    int pH = INT_ARG(4);                                                        // paddings height
    int pW = INT_ARG(5);                                                        // paddings width
    int dH = INT_ARG(6);                                                        // dilations height
    int dW = INT_ARG(7);                                                        // dilations width
    int isSameMode = INT_ARG(8);                                                // 0-VALID, 1-SAME
    int isNCHW  = block.getIArguments()->size() > 9 ? !INT_ARG(9) : 1;          // 0-NHWC, 1-NCHW    

    REQUIRE_TRUE(input->rankOf()   == 4, 0, "CUSTOM CONV2D_BP OP: rank of input array must be equal to 4, but got %i instead !", input->rankOf());
    REQUIRE_TRUE(weights->rankOf() == 4, 0, "CUSTOM CONV2D_BP OP: rank of weights array must be equal to 4, but got %i instead !", weights->rankOf());
    REQUIRE_TRUE(gradO->rankOf() == 4, 0, "CUSTOM CONV2D_BP OP: rank of output's gradients (next epsilon) array must be equal to 4, but got %i instead !", gradO->rankOf());

    int bS, iC, iH, iW, oC, oH, oW;                             // batch size, input channels, input height/width, output channels, output height/width;
    int indIOioC, indIiH, indWoC, indWiC, indWkH, indOoH;       // corresponding indexes
    ConvolutionUtils<T>::getSizesAndIndexesConv2d(isNCHW, *input, *gradO, bS, iC, iH, iW, oC, oH, oW, indIOioC, indIiH, indWiC, indWoC, indWkH, indOoH);

    int trueoH, trueoW;          // true output height, width
    ConvolutionUtils<T>::calcOutSizePool2D(trueoH, trueoW, kH, kW, sH, sW, pH, pW, dH, dW, iH, iW, isSameMode);

    std::string expectedGradOShape   = ShapeUtils<T>::shapeAsString(ShapeUtils<T>::composeShapeUsingDimsAndIdx({bS,oC,trueoH,trueoW,  0,indIOioC,indOoH,indOoH+1}));            
    std::string expectedWeightsShape = ShapeUtils<T>::shapeAsString(ShapeUtils<T>::composeShapeUsingDimsAndIdx({iC,oC,kH,kW,  indWiC,indWoC,indWkH,indWkH+1}));
    REQUIRE_TRUE(expectedGradOShape == ShapeUtils<T>::shapeAsString(gradO), 0,  "CUSTOM CONV2D_BP OP: wrong shape of output gradients (next epsilon) array, expected is %s, but got %s instead !", expectedGradOShape.c_str(), ShapeUtils<T>::shapeAsString(gradO).c_str());
    REQUIRE_TRUE(expectedWeightsShape == ShapeUtils<T>::shapeAsString(weights), 0, "CUSTOM CONV2D_BP OP: wrong shape of weights array, expected is %s, but got %s instead !", expectedWeightsShape.c_str(), ShapeUtils<T>::shapeAsString(weights).c_str());    
    if(bias)
        REQUIRE_TRUE(bias->rankOf() <= 2 && oC == bias->lengthOf(), 0, "CUSTOM CONV2D_BP OP: wrong shape of array with biases, expected rank, length: <=2, %i, but got %i, %i instead !", oC, bias->rankOf(), bias->lengthOf());

    ConvolutionUtils<T>::conv2dBP({input, weights, bias, gradO}, {gradI, gradW, gradB}, {kH,kW,sH,sW,pH,pW,dH,dW,isSameMode,isNCHW});
    
    return Status::OK();
}



DECLARE_SHAPE_FN(conv2d_bp) {

    auto inputShapeInfo   = inputShape->at(0);                                                // [bS, iH, iW, iC] (NHWC) or [bS, iC, iH, iW] (NCHW)
    auto weightsShapeInfo = inputShape->at(1);                                                // [kH, kW, iC, oC] (NHWC) or [oC, iC, kH, kW] (NCHW)
    auto biasShapeInfo    = block.width() > 3 ? inputShape->at(2) : nullptr;                  // [oC]
    auto gradOShapeInfo   = block.width() > 3 ? inputShape->at(3) : inputShape->at(2);        // [bS, oH, oW, oC] (NHWC) or [bS, oC, oH, oW] (NCHW), epsilon_next

    const int rank = 4;

    REQUIRE_TRUE(inputShapeInfo[0]   == rank, 0, "CUSTOM CONV2D_BP OP: rank of input array must be equal to %i, but got %i instead !", rank, inputShapeInfo[0]);
    REQUIRE_TRUE(weightsShapeInfo[0] == rank, 0, "CUSTOM CONV2D_BP OP: rank of weights array must be equal to %i, but got %i instead !", rank, weightsShapeInfo[0]);
    REQUIRE_TRUE(gradOShapeInfo[0]   == rank, 0, "CUSTOM CONV2D_BP OP: rank of output gradients (next epsilon) array must be equal to %i, but got %i instead !", rank, gradOShapeInfo[0]);

    const int kH = INT_ARG(0);                                                        // filter(kernel) height
    const int kW = INT_ARG(1);                                                        // filter(kernel) width
    const int sH = INT_ARG(2);                                                        // strides height
    const int sW = INT_ARG(3);                                                        // strides width
    const int pH = INT_ARG(4);                                                        // paddings height
    const int pW = INT_ARG(5);                                                        // paddings width
    const int dH = INT_ARG(6);                                                        // dilations height
    const int dW = INT_ARG(7);                                                        // dilations width
    const int isSameMode = INT_ARG(8);                                                // 0-VALID, 1-SAME
    const int isNCHW  = block.getIArguments()->size() > 9 ? !INT_ARG(9) : 1;          // 0-NHWC, 1-NCHW    

    int indIOioC, indIiH, indWkH, indWoC, indWiC, indOoH;
    if(!isNCHW) {
        indIOioC = 3; indIiH = 1; indWkH = 0; indWoC = 3; indWiC = 2, indOoH = 1;
    }
    else {        
        indIOioC = 1; indIiH = 2; indWkH = 2; indWoC = 0; indWiC = 1, indOoH = 2;              
    }    

    const int bS = inputShapeInfo[1];                            // batch size
    const int iH = inputShapeInfo[indIiH+1];                     // input height
    const int iW = inputShapeInfo[indIiH+2];                     // input width
    const int iC = inputShapeInfo[indIOioC+1];                   // input channels        
    const int oC = weightsShapeInfo[indWoC+1];                   // output channels

    int trueoH, trueoW;          // true output height, width
    ConvolutionUtils<T>::calcOutSizePool2D(trueoH, trueoW, kH, kW, sH, sW, pH, pW, dH, dW, iH, iW, isSameMode);

    std::string expectedGradOShape   = ShapeUtils<T>::shapeAsString(ShapeUtils<T>::composeShapeUsingDimsAndIdx({bS,oC,trueoH,trueoW,  0,indIOioC,indOoH,indOoH+1}));            
    std::string expectedWeightsShape = ShapeUtils<T>::shapeAsString(ShapeUtils<T>::composeShapeUsingDimsAndIdx({iC,oC,kH,kW,  indWiC,indWoC,indWkH,indWkH+1}));
    REQUIRE_TRUE(expectedGradOShape == ShapeUtils<T>::shapeAsString(gradOShapeInfo), 0,  "CUSTOM CONV2D_BP OP: wrong shape of output gradients (next epsilon) array, expected is %s, but got %s instead !", expectedGradOShape.c_str(), ShapeUtils<T>::shapeAsString(gradOShapeInfo).c_str());
    REQUIRE_TRUE(expectedWeightsShape == ShapeUtils<T>::shapeAsString(weightsShapeInfo), 0, "CUSTOM CONV2D_BP OP: wrong shape of weights array, expected is %s, but got %s instead !", expectedWeightsShape.c_str(), ShapeUtils<T>::shapeAsString(weightsShapeInfo).c_str());
    if(biasShapeInfo)
        REQUIRE_TRUE(biasShapeInfo[0] <= 2 && oC == shape::length(biasShapeInfo), 0, "CUSTOM CONV2D_BP OP: wrong shape of array with biases, expected rank, length: <=2, %i, but got %i, %i instead !", oC, biasShapeInfo[0], shape::length(biasShapeInfo));
    
    Nd4jLong* gradIshapeInfo(nullptr), *gradWshapeInfo(nullptr);
    COPY_SHAPE(inputShapeInfo, gradIshapeInfo);
    COPY_SHAPE(weightsShapeInfo, gradWshapeInfo);

    if(biasShapeInfo) {
        Nd4jLong* gradBshapeInfo(nullptr);
        COPY_SHAPE(biasShapeInfo, gradBshapeInfo);
        return SHAPELIST(gradIshapeInfo, gradWshapeInfo, gradBshapeInfo);
    }     

    return SHAPELIST(gradIshapeInfo, gradWshapeInfo);        
}


}
}

#endif

#endif //LIBND4J_CONVO_OPS_H
