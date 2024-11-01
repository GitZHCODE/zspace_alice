// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Taizhong Chen <taizhong.chen@zaha-hadid.com>
//


#include <userSrc/tzSketches/OV/zTsOVdataProcessor.h>

namespace zSpace
{
	// Public

	zTsOVdataProcessor::zTsOVdataProcessor() {};
	zTsOVdataProcessor::~zTsOVdataProcessor() {};

	void zTsOVdataProcessor::initialise(string ov_destinationPath, string in_usdPath, string rootPrimName)
	{
		// connect to ov
		//if(init_connect_ov(ov_destinationPath))
		init_connect_ov(ov_destinationPath);

		if (stage = UsdStage::Open(in_usdPath))
		{
			LOG_BUILD << "Successfully read usd file: " << in_usdPath << endl;

			// store prims
			for (UsdPrim prim : stage->Traverse())
			{
				if (prim.IsA<UsdGeomMesh>())
				{
					UsdGeomMesh usd(prim);
					bool chk = usd.GetDisplayColorAttr().Clear();

					primArr.push_back(prim);
				}
			}

			LOG_BUILD << primArr.size() << " mesh(es) found" << endl;

			// create zObjMeshes
			oMeshArr.assign(primArr.size(), zObjMesh());
			for (int i = 0; i < primArr.size(); i++)
			{
				zFnMesh fn(oMeshArr[i]);
				fn.from(primArr[i]);
			}
		}
		else
			LOG_BUILD << "Unable to read usd file: " << in_usdPath << endl;

#ifdef MAKE_VARIANT
		for (UsdPrim prim : stage->Traverse())
			if (prim.GetName() == rootPrimName)
			{
				rootPrim = prim;
				//create default variant
				UsdVariantSets variantSets = rootPrim.GetVariantSets();
				UsdVariantSet geomVariantSet = variantSets.AddVariantSet("zSpace");
				geomVariantSet.AddVariant("default");
				geomVariantSet.SetVariantSelection("default");

				LOG_BUILD << "rootPrim found: " << rootPrimName << endl
					<< "VariantSet has been turned on" << endl;
				break;
			}
#endif

	}

	void zTsOVdataProcessor::save()
	{
		if(stage->GetRootLayer()->Save())
			LOG_BUILD << "Successfully saved to usd file: " << stage->GetRootLayer()->GetIdentifier() << endl;
	}

	void zTsOVdataProcessor::exportTo(string out_usdPath)
	{
		if (stage->GetRootLayer()->Export(out_usdPath))
			LOG_BUILD << "Successfully export to usd file: " << out_usdPath << endl;
	}

	void zTsOVdataProcessor::draw(bool dVertex, bool dEdges, bool dFaces)
	{
		for (auto& m : oMeshArr)
		{
			m.setDisplayElements(dVertex, dEdges, dFaces);
			m.draw();
		}
	}

	void zTsOVdataProcessor::getRawMesh(zObjMesh* mesh, int numMeshes)
	{
		mesh = &oMeshArr[0];
		numMeshes = oMeshArr.size();
	}

	void zTsOVdataProcessor::getMeshArray(zObjMeshArray& meshes)
	{
		meshes = oMeshArr;
	}

	void zTsOVdataProcessor::compute(zProcessType type, bool updateMeshColour)
	{

#ifdef MAKE_VARIANT
		// create variant
		UsdVariantSets variantSets = rootPrim.GetVariantSets();
		UsdVariantSet variantSet = variantSets.GetVariantSet("zSpace");
		if (!variantSet.IsValid()) variantSets.AddVariantSet("zSpace");
#endif

		switch (type)
		{
		case zProcessType::meshId:
		{
#ifdef MAKE_VARIANT
			// add variant
			variantSet.AddVariant("v_blockId");
			variantSet.SetVariantSelection("v_blockId");
			UsdEditContext editContext(stage, variantSet.GetVariantEditTarget());
			LOG_DEV << variantSet.GetVariantSelection() << endl;
#endif
			compute_id(updateMeshColour);
			break;
		}
		case zProcessType::meshVolume:
		{
#ifdef MAKE_VARIANT
			// add variant
			variantSet.AddVariant("v_blockVolume");
			variantSet.SetVariantSelection("v_blockVolume");
			UsdEditContext editContext(stage, variantSet.GetVariantEditTarget());
			LOG_DEV << variantSet.GetVariantSelection() << endl;
#endif
			compute_volume(updateMeshColour);
			break;
		}
		case zProcessType::meshPlanarity:
		{
#ifdef MAKE_VARIANT
			// add variant
			variantSet.AddVariant("v_facePlanarity");
			variantSet.SetVariantSelection("v_facePlanarity");
			UsdEditContext editContext(stage, variantSet.GetVariantEditTarget());
			LOG_DEV << variantSet.GetVariantSelection() << endl;
#endif
			compute_planarity(updateMeshColour);
			break;
		}
		default:
			break;
		}

#ifdef MAKE_VARIANT
		variantSet.SetVariantSelection("default");
#endif

	}

	void zTsOVdataProcessor::compute_id(bool updateMeshColour)
	{
		for (int i = 0; i < primArr.size(); i++)
		{
			addProperty(primArr[i], TfToken("blockId"), SdfValueTypeNames->Int, i);
		}

		if (updateMeshColour)
		{
			zDomainFloat inDomain(0, primArr.size());
			zDomainColor outDomain(zRED, zBLUE);

			for (int i = 0; i < primArr.size(); i++)
			{
				zColor col = zspaceCore.blendColor(i, inDomain, outDomain, zColorType::zRGB);
				//zColor col = generateRandomColour();
				//cout << "color: " << col.r << " " << col.g << " " << col.b << endl;
					
				// update obj color
				zFnMesh fn(oMeshArr[i]);
				fn.setFaceColor(col, true);

				// update usd color
				UsdGeomMesh usd(primArr[i]);
				VtArray<GfVec3f> vt_cols = { GfVec3f(col.r, col.g, col.b) };
				usd.GetDisplayColorPrimvar().Set(vt_cols);
				usd.GetDisplayColorPrimvar().SetIndices(VtArray{ 0 });
				
				//SdfPath primPath = primArr[i].GetPrimPath();
				//TfToken primName = primArr[i].GetName();
				//UsdPrim newPrim = stage->DefinePrim(primPath, primName);
				//fn.to(newPrim);
			}
		}
	}

	void zTsOVdataProcessor::compute_volume(bool updateMeshColour)
	{
		double volume = 0;
		vector<double> volumes;

		for (int i = 0; i < primArr.size(); i++)
		{
			zFnMesh fn(oMeshArr[i]);
			volume = fn.getMeshVolume();

			volumes.push_back(volume);
			addProperty(primArr[i], TfToken("blockVolume"), SdfValueTypeNames->Double, volume);
		}

		if (updateMeshColour)
		{
			zDomainFloat inDomain(zspaceCore.zMin(volumes), zspaceCore.zMax(volumes));
			zDomainColor outDomain(zWHITE, zBLACK);

			for (int i = 0; i < primArr.size(); i++)
			{
				zColor col = zspaceCore.blendColor(volumes[i], inDomain, outDomain, zColorType::zRGB);

				// update obj color
				zFnMesh fn(oMeshArr[i]);
				fn.setFaceColor(col, true);

				// update usd color
				UsdGeomMesh usd(primArr[i]);
				VtArray<GfVec3f> vt_cols = { GfVec3f(col.r, col.g, col.b) };
				usd.GetDisplayColorPrimvar().Set(vt_cols);
				usd.GetDisplayColorPrimvar().SetIndices(VtArray{ 0 });
			}
		}
	}

	void zTsOVdataProcessor::compute_planarity(bool updateMeshColour)
	{
		vector<double> planarityDevs;
		for (int i = 0; i < primArr.size(); i++)
		{
			zObjMesh temp;
			zFnMesh fn(oMeshArr[i]);
			fn.getPlanarityDeviationPerFace(planarityDevs, zPlanarSolverType::zVolumePlanar);

			addProperty(primArr[i], TfToken("blockFacePlanarity"), SdfValueTypeNames->DoubleArray, VtArray<double>(planarityDevs.begin(), planarityDevs.end()));
		}

		if (updateMeshColour)
		{
			for (int i = 0; i < primArr.size(); i++)
			{
				// update obj color
				zFnMesh fn(oMeshArr[i]);

				int numF = fn.numPolygons();
				zColor* rawFColour = fn.getRawFaceColors();
				updateColour_unique(numF, rawFColour, primArr[i]);
			}
		}
	}

	zColor zTsOVdataProcessor::generateRandomColour()
	{
		srand(time(0)); // Seed for randomness

		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

		return zColor(r, g, b);
	}


	// Private

	// usd methods

	template <typename T>
	void zTsOVdataProcessor::addProperty(UsdPrim& usd, TfToken& token, SdfValueTypeName typeName, const T& propertyValue, UsdTimeCode timeCode)
	{
		UsdGeomPrimvarsAPI primvar(usd);

		if (timeCode.IsDefault() || timeCode == 0)
		{
			primvar.CreatePrimvar(token, typeName).Set(propertyValue, timeCode);
		}
		else
		{
			primvar.GetPrimvar(token).Set(propertyValue, timeCode);
		}
	}


	void zTsOVdataProcessor::updateColour_unique(int numColours, zColor* cols, UsdPrim& prim)
	{
		UsdGeomMesh usd(prim);

		VtArray<GfVec3f> fCols;
		VtArray<float> opacity;

		VtArray<GfVec3f> fCols_unique;
		VtArray<float> opacity_unique;
		VtArray<int> fCols_unique_index;


		// v positions and color
		for (int i = 0; i < numColours; i++)
		{
			GfVec3f c_attr;

			opacity.push_back(1.0);
			c_attr.Set(cols[i].r, cols[i].g, cols[i].b);
			fCols.push_back(c_attr);

			int id = -1;
			bool chkRepeat = false;

			for (int j = 0; j < fCols_unique.size(); j++)
			{
				if (fCols_unique[j] == c_attr)
				{
					chkRepeat = true;
					id = j;
					break;
				}
			}

			if (chkRepeat)
			{
				fCols_unique_index.push_back(id);
			}
			else
			{
				fCols_unique_index.push_back(fCols_unique.size());
				fCols_unique.push_back(c_attr);
				opacity_unique.push_back(1.0);

			}
		}

		//update display color
		TfToken interpolationType = pxr::UsdGeomTokens->uniform;

		usd.GetDisplayColorPrimvar().Set(fCols_unique);
		usd.GetDisplayColorPrimvar().SetIndices(fCols_unique_index);
		usd.GetDisplayColorPrimvar().SetInterpolation(interpolationType);
	}

	// ov methods
	bool zTsOVdataProcessor::init_connect_ov(string destinationPath)
	{
		// Connect to OV
		bool doLiveEdit = false;
		std::string existingStage;
		bool chk = omniCore.isValidOmniURL(destinationPath);

		omniCore.printConnectedUsername(destinationPath);

		return omniCore.startOmniverse();
	}

	void zTsOVdataProcessor::shundown()
	{
		// All done, shut down our connection to Omniverse
		omniCore.shutdownOmniverse();
		LOG_BUILD << "Shut down Omniverse" << endl;

	}

}