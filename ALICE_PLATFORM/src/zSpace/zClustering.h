#pragma once

#include <zSpace/zMesh.h>

namespace zSpace
{
		
	/*! \brief This method computes the best fit plane for the given points using PCA
	*
	*	\param		[in]	points			- input points.
	*	\return 			Best fit zPlane.
	*/
	vector<zCluster> zClustering_BoundingBox(zMesh &operateMesh, zVector& boundingDims);


	
}

