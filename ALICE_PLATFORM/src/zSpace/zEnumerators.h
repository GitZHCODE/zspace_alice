#pragma once
#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif


namespace zSpace
{
	enum zSPACE_API zHEData { zVertexData = 0, zEdgeData, zFaceData};

	enum zSPACE_API zAttributeData { zColorAttribute=10, zNormalAttribute, zWeightAttribute, zCurvatureAttribute,zHeightAttribute, zVectorAttribute, zIntAttribute, zDoubleAttribute, zBooleanAttribute};

	enum zSPACE_API zScalarfieldData { zGradientDescent=30, zGradientAscent };

	enum zSPACE_API zColorData { zRGB = 40, zHSV };

	enum zSPACE_API zDataLevel { zLsoaData = 50, zMsoaData, zPostcodeData };

	enum zSPACE_API zIntergrationType { zEuler = 60, zRK4 , zPixel };

	enum zSPACE_API zDiffusionType { zLaplacian = 70, zAverage, zLaplacianAdjacent};

	enum zSPACE_API zSlimeParameter { zSO = 80, zSA, zRA, zdepT};

}
