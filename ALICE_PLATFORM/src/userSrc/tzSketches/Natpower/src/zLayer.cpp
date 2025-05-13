#include <userSrc/tzSketches/Natpower/include/zLayer.h>

namespace zSpace
{
	zLayer::zLayer()
	{
		sectionMesh = new zObjMesh();
		sectionGraph = new zObjGraph();

		unrolledMesh = new zObjMesh();
		unrolledGraph = new zObjGraph();

		sectionCablePos = new zPoint();
		unrolledCablePos = new zPoint();
	}

	zLayer::~zLayer()
	{

	}

	void zLayer::drawSectionMesh()
	{
		sectionMesh->draw();
	}

	void zLayer::drawUnrolledMesh()
	{
		unrolledMesh->draw();
	}

	void zLayer::drawSectionGraph()
	{
		sectionGraph->draw();
	}	
	
	void zLayer::drawUnrolledGraph()
	{
		unrolledGraph->draw();
	}

	void zLayer::setDisplay()
	{
		sectionMesh->setDisplayElements(false, true, true);
		sectionGraph->setDisplayElements(false, true);
		unrolledMesh->setDisplayElements(false, true, false);
		unrolledGraph->setDisplayElements(false, true);

		zFnGraph fnGraph(*sectionGraph);
		fnGraph.setEdgeColor(zRED);
		fnGraph.setEdgeWeight(2.0);

		fnGraph = zFnGraph(*unrolledGraph);
		fnGraph.setEdgeColor(zRED);
		fnGraph.setEdgeWeight(2.0);

		//for (zItGraphEdge e(*sectionGraph); !e.end(); e++)
		//{
		//	if (e.getHalfEdge(0).onBoundary() || e.getHalfEdge(1).onBoundary())
		//	{
		//		e.setWeight(4.0);
		//		e.setColor(zRED);
		//	}
		//	else
		//	{
		//		e.setWeight(2.0);
		//		e.setColor(zGREEN);
		//	}
		//}
	}

}