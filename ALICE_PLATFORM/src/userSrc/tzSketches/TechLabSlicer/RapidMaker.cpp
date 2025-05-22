#include "RapidMaker.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>

namespace zSpace
{
	namespace ABB
	{
		// Pos implementations
		Pos Pos::fromMatrix(const Matrix4d& mat) {
			return Pos(mat(0, 3), mat(1, 3), mat(2, 3));
		}

		string Pos::toRapidString() const {
			return "[" + to_string(x) + "," + to_string(y) + "," + to_string(z) + "]";
		}

		// Orient implementations
		Orient Orient::fromMatrix(const Matrix4d& mat) {
			Matrix3d rotMat = mat.block<3, 3>(0, 0);
			Quaterniond q(rotMat);
			return Orient(q.w(), q.x(), q.y(), q.z());
		}

		string Orient::toRapidString() const {
			return "[" + to_string(q1) + "," + to_string(q2) + ","
				+ to_string(q3) + "," + to_string(q4) + "]";
		}

		// WObjData implementation
		string WObjData::toRapidString() const {
			return "[" + string(robhold ? "TRUE" : "FALSE") + ","
				+ string(ufprog ? "TRUE" : "FALSE") + ",\"" + ufmec + "\","
				+ "[" + trans.toRapidString() + "," + rot.toRapidString() + "],"
				+ "[" + oframe.toRapidString() + "," + oframe_rot.toRapidString() + "]]";
		}

		// ToolData implementation
		string ToolData::toRapidString() const {
			return "[" + string(robhold ? "TRUE" : "FALSE") + ", "
				+ "[" + tframe.toRapidString() + ", " + tframe_rot.toRapidString() + "], "
				+ "[" + to_string(mass) + ", " + cog.toRapidString() + ", "
				+ inertia.toRapidString() + ", "
				+ to_string(aios) + ", " + to_string(aios_unit) + ", "
				+ to_string(aios_value) + "]]";
		}

		// ZoneData implementation
		string ZoneData::toRapidString() const {
			return "[" + string(finep ? "TRUE" : "FALSE") + ", "
				+ to_string(pzone_tcp) + ", " + to_string(pzone_ori) + ", "
				+ to_string(pzone_eax) + ", " + to_string(zone_ori) + ", "
				+ to_string(zone_leax) + ", " + to_string(zone_reax) + "]";
		}

		// ConfData implementation
		string ConfData::toRapidString() const {
			return "[" + to_string(cf1) + ", " + to_string(cf4) + ", "
				+ to_string(cf6) + ", " + to_string(cfx) + "]";
		}

		// Target implementations
		Target::Target(const Matrix4d& trans, const string& targetName)
			: transform(trans), name(targetName), confData(ConfData()) {}

		Target::Target(const Matrix4d& trans, const ConfData& config, const string& targetName)
			: transform(trans), name(targetName), confData(config) {}

		const Matrix4d& Target::getTransform() const { return transform; }
		const string& Target::getName() const { return name; }
		const ConfData& Target::getConfig() const { return confData; }

		void Target::setTransform(const Matrix4d& trans) { transform = trans; }
		void Target::setName(const string& targetName) { name = targetName; }
		void Target::setConfig(const ConfData& config) { confData = config; }

		Target Target::withZ(double newZ) const {
			Matrix4d newTransform = transform;
			newTransform(2, 3) = newZ;
			return Target(newTransform, confData, name);
		}

		string Target::toRapidString() const {
			// Get position from the last row of the transform matrix
			Vector3d pos = transform.block<3, 1>(0, 3);

			// Get rotation matrix and convert to quaternion
			Matrix3d rotMat = transform.block<3, 3>(0, 0);
			Quaterniond q(rotMat);
			q.normalize();  // Ensure quaternion is normalized

			stringstream ss;
			ss << "[[" << pos.x() << ", " << pos.y() << ", " << pos.z() << "], "
				<< "[" << q.w() << ", " << q.x() << ", "
				<< q.y() << ", " << q.z() << "], "
				<< confData.toRapidString() << ", "
				<< "[9E9, 9E9, 9E9, 9E9, 9E9, 9E9]]";
			return ss.str();
		}

		// Procedure implementations
		Procedure::Procedure(const string& procName)
			: name(procName), currentLayer(0), currentLineType("NONE") {}

		void Procedure::addCommand(const string& cmd) {
			commands.push_back(cmd);
		}

		void Procedure::addMoveL(const Target& target, double speed,
			const string& zoneRef,
			const string& toolRef,
			const string& wobjRef) {
			try {
				if (zoneRef.empty() || toolRef.empty() || wobjRef.empty()) {
					cerr << "Error: Empty reference name passed to addMoveL" << endl;
					return;
				}

				stringstream ss;
				ss << "        MoveL " << target.toRapidString()
					<< ", [" << speed << ", 500, 5000, 1000], "
					<< zoneRef << ", " << toolRef << ", \\Wobj:=" << wobjRef << ";";
				commands.push_back(ss.str());
			}
			catch (const exception& e) {
				cerr << "Error in addMoveL: " << e.what() << endl;
			}
		}

		void Procedure::addMoveAbsJ(const vector<double>& jRotations, double speed,
			const string& zoneRef,
			const string& toolRef,
			const string& wobjRef) {
			try {
				if (jRotations.size() != 6) {
					cerr << "Error: Joint rotations must be 6 to passed to addMoveAbsJ" << endl;
					return;
				}

				stringstream ss;
				ss << "        MoveAbsJ " << "[["
					<< jRotations[0] << ", "
					<< jRotations[1] << ", "
					<< jRotations[2] << ", "
					<< jRotations[3] << ", "
					<< jRotations[4] << ", "
					<< jRotations[5] << "]"
					<< ", [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]]"
					<< ", [" << speed << ", 500, 5000, 1000], "
					<< zoneRef << ", " << toolRef << ";";
				commands.push_back(ss.str());
			}
			catch (const exception& e) {
				cerr << "Error in addMoveAbsJ: " << e.what() << endl;
			}
		}

		void Procedure::addSetDO(const string& signal, bool value) {
			commands.push_back("        SetDO " + signal + ", " + (value ? "1" : "0") + ";");
		}

		void Procedure::addSetAO(const string& signal, double value) {
			commands.push_back("        SetAO " + signal + ", " + to_string(value) + ";");
		}

		void Procedure::addWaitTime(double seconds) {
			commands.push_back("        WaitTime " + to_string(seconds) + ";");
		}

		void Procedure::setLayer(int layer) {
			currentLayer = layer;
			addCommand("        !LAYER " + to_string(layer));
		}

		void Procedure::setLineType(const string& type) {
			currentLineType = type;
			addCommand("        !LINETYPE " + type);
		}

		const string& Procedure::getName() const { return name; }

		string Procedure::toRapidString() const {
			stringstream ss;
			ss << "    PROC " << name << "()\n";
			for (const auto& cmd : commands) {
				ss << cmd << "\n";
			}
			ss << "    ENDPROC\n";
			return ss.str();
		}

		// Module implementations
		string Module::RapidData::getDeclaration() const {
			string typeStr;
			switch (type) {
			case RapidDataType::PERS: typeStr = "PERS"; break;
			case RapidDataType::VAR: typeStr = "VAR"; break;
			case RapidDataType::CONSTANT: typeStr = "CONSTANT"; break;
			}

			string classStr;
			switch (dataClass) {
			case RapidDataClass::WOBJDATA: classStr = "wobjdata"; break;
			case RapidDataClass::TOOLDATA: classStr = "tooldata"; break;
			case RapidDataClass::ZONEDATA: classStr = "zonedata"; break;
			case RapidDataClass::CONFDATA: classStr = "confdata"; break;
			case RapidDataClass::NUM: classStr = "num"; break;
			case RapidDataClass::BOOL: classStr = "bool"; break;
			case RapidDataClass::STRING: classStr = "string"; break;
			}

			return typeStr + " " + classStr;
		}

		Module::Module(const string& modName, const PrinterSettings& printerSettings)
			: name(modName), settings(printerSettings) {
			// Add default data
			addWObjData(WObjData());  // Uses default name
			addToolData(ToolData());   // Uses default name
			addZoneData(ZoneData());   // Uses default name
			addConfData(ConfData());   // Uses default name
		}

		// Helper methods to get or create data
		const WObjData& Module::getOrDefault(const WObjData* data) const {
			if (data) {
				auto it = wobjData.find(data->name);
				return (it != wobjData.end()) ? it->second : findWObjData("defaultWObj");
			}
			return findWObjData("defaultWObj");
		}

		const ToolData& Module::getOrDefault(const ToolData* data) const {
			if (data) {
				auto it = toolData.find(data->name);
				return (it != toolData.end()) ? it->second : findToolData("defaultTool");
			}
			return findToolData("defaultTool");
		}

		const ZoneData& Module::getOrDefault(const ZoneData* data) const {
			if (data) {
				auto it = zoneData.find(data->name);
				return (it != zoneData.end()) ? it->second : findZoneData("defaultZone");
			}
			return findZoneData("defaultZone");
		}

		const ConfData& Module::getOrDefault(const ConfData* data) const {
			if (data) {
				auto it = confData.find(data->name);
				return (it != confData.end()) ? it->second : findConfData("defaultConfig");
			}
			return findConfData("defaultConfig");
		}

		void Module::updateRapidData(const WObjData& data) {
			wobjDataList[data.name] = RapidData(data.name, RapidDataType::PERS, RapidDataClass::WOBJDATA, data.toRapidString());
		}

		void Module::updateRapidData(const ToolData& data) {
			toolDataList[data.name] = RapidData(data.name, RapidDataType::PERS, RapidDataClass::TOOLDATA, data.toRapidString());
		}

		void Module::updateRapidData(const ZoneData& data) {
			zoneDataList[data.name] = RapidData(data.name, RapidDataType::PERS, RapidDataClass::ZONEDATA, data.toRapidString());
		}

		void Module::updateRapidData(const ConfData& data) {
			confDataList[data.name] = RapidData(data.name, RapidDataType::PERS, RapidDataClass::CONFDATA, data.toRapidString());
		}

		void Module::addWObjData(const WObjData& data) {
			wobjData[data.name] = data;
			updateRapidData(data);
		}

		void Module::addToolData(const ToolData& data) {
			toolData[data.name] = data;
			updateRapidData(data);
		}

		void Module::addZoneData(const ZoneData& data) {
			zoneData[data.name] = data;
			updateRapidData(data);
		}

		void Module::addConfData(const ConfData& data) {
			confData[data.name] = data;
			updateRapidData(data);
		}

		const WObjData& Module::findWObjData(const string& name) const {
			auto it = wobjData.find(name);
			if (it != wobjData.end()) {
				return it->second;
			}
			static const WObjData defaultData;  // Return default if not found
			return defaultData;
		}

		const ToolData& Module::findToolData(const string& name) const {
			auto it = toolData.find(name);
			if (it != toolData.end()) {
				return it->second;
			}
			static const ToolData defaultData;
			return defaultData;
		}

		const ZoneData& Module::findZoneData(const string& name) const {
			auto it = zoneData.find(name);
			if (it != zoneData.end()) {
				return it->second;
			}
			static const ZoneData defaultData;
			return defaultData;
		}

		const ConfData& Module::findConfData(const string& name) const {
			auto it = confData.find(name);
			if (it != confData.end()) {
				return it->second;
			}
			static const ConfData defaultData;
			return defaultData;
		}

		void Module::addMoveL(Procedure& proc, const Target& target, double speed,
			const WObjData* wobj,
			const ToolData* tool,
			const ZoneData* zone) {
			try {
				const WObjData& wobjData = getOrDefault(wobj);
				const ToolData& toolData = getOrDefault(tool);
				const ZoneData& zoneData = getOrDefault(zone);

				if (wobjData.name.empty() || toolData.name.empty() || zoneData.name.empty()) {
					cerr << "Error: Invalid data names in Module::addMoveL" << endl;
					return;
				}

				proc.addMoveL(target, speed, zoneData.name, toolData.name, wobjData.name);
			}
			catch (const exception& e) {
				cerr << "Error in Module::addMoveL: " << e.what() << endl;
			}
		}

		void Module::addMoveAbsJ(Procedure& proc, const vector<double>& jRotations, double speed,
			const WObjData* wobj,
			const ToolData* tool,
			const ZoneData* zone){
			try {
				const WObjData& wobjData = getOrDefault(wobj);
				const ToolData& toolData = getOrDefault(tool);
				const ZoneData& zoneData = getOrDefault(zone);

				if (wobjData.name.empty() || toolData.name.empty() || zoneData.name.empty()) {
					cerr << "Error: Invalid data names in Module::addMoveAbsJ" << endl;
					return;
				}

				proc.addMoveAbsJ(jRotations, speed, zoneData.name, toolData.name, wobjData.name);
			}
			catch (const exception& e) {
				cerr << "Error in Module::addMoveAbsJ: " << e.what() << endl;
			}

		}

		const string& Module::getName() const { return name; }

		Procedure& Module::createProcedure(const string& procName) {
			procedures.emplace_back(procName);
			return procedures.back();
		}

		Procedure& Module::getProcedure(const string& procName) {
			for (auto& proc : procedures) {
				if (proc.getName() == procName) {
					return proc;
				}
			}
			return createProcedure(procName);
		}

		void Module::setSettings(const PrinterSettings& newSettings) {
			settings = newSettings;
		}

		const PrinterSettings& Module::getSettings() const { return settings; }

		void Module::writeRapidPersistentData(stringstream& ss) const {
			for (const auto& [name, data] : wobjDataList) {
				ss << "    " << data.getDeclaration() << " " << name << " := " << data.content << ";\n";
			}
			for (const auto& [name, data] : toolDataList) {
				ss << "    " << data.getDeclaration() << " " << name << " := " << data.content << ";\n";
			}
			for (const auto& [name, data] : zoneDataList) {
				ss << "    " << data.getDeclaration() << " " << name << " := " << data.content << ";\n";
			}
			for (const auto& [name, data] : confDataList) {
				ss << "    " << data.getDeclaration() << " " << name << " := " << data.content << ";\n";
			}
			ss << "\n";
		}

		string Module::toRapidString() const {
			stringstream ss;
			ss << "MODULE " << name << "\n";
			writeRapidPersistentData(ss);
			for (const auto& proc : procedures) {
				ss << proc.toRapidString() << "\n";
			}
			ss << "ENDMODULE\n";
			return ss.str();
		}

		bool Module::saveToFile(const string& filepath) const {
			try {
				ofstream file(filepath);
				if (!file.is_open()) {
					cerr << "Failed to open file: " << filepath << endl;
					return false;
				}
				file << toRapidString();
				file.close();

				if (!file.good()) {
					cerr << "Error occurred while writing to file: " << filepath << endl;
					return false;
				}
				return true;
			}
			catch (const exception& e) {
				cerr << "Exception while saving file " << filepath << ": " << e.what() << endl;
				return false;
			}
		}

		// RapidMaker implementations
		RapidMaker::RapidMaker() {}

		Module& RapidMaker::createModule(const string& name, const PrinterSettings& settings) {
			modules.emplace_back(name, settings);
			return modules.back();
		}

		Module& RapidMaker::getModule(const string& name) {
			for (auto& mod : modules) {
				if (mod.getName() == name) {
					return mod;
				}
			}
			return createModule(name);
		}

		void RapidMaker::addPrintMove(Module& module, Procedure& proc, const vector<Target>& targets,
			const ZoneData* zone,
			const ToolData* tool,
			const WObjData* wobj) {
			if (targets.empty()) return;

			const auto& settings = module.getSettings();

			// Start extrusion before the first move
			proc.addSetDO(RapidSignals::START_EXTRUSION, true);
			proc.addSetAO(RapidSignals::EXTRUSION_RATE, settings.extrusionRate);

			// Add all movement commands
			for (const auto& target : targets) {
				module.addMoveL(proc, target, settings.printSpeed, wobj, tool, zone);
			}
		}

		void RapidMaker::addPrintMoveWithApproach(Module& module, Procedure& proc,
			const vector<Target>& targets,
			double approachHeight,
			const ZoneData* zone,
			const ToolData* tool,
			const WObjData* wobj) {
			if (targets.empty()) return;

			const auto& settings = module.getSettings();


			// Move to approach position above first point
			Target approachTarget = targets.front().withZ(
				targets.front().getTransform()(2, 3) + approachHeight
			);

			addTravelMove(module, proc, approachTarget, zone, tool, wobj);

			// Move down to start position
			module.addMoveL(proc, targets.front(), settings.printSpeed, wobj, tool, zone);

			// Start printing
			addPrintMove(module, proc, targets, zone, tool, wobj);

			// Lift after printing
			Target liftTarget = targets.back().withZ(
				targets.back().getTransform()(2, 3) + approachHeight
			);

			module.addMoveL(proc, liftTarget, settings.printSpeed, wobj, tool, zone);

			//// Move to approach position above first point
			//Target approachTarget = targets.front().withZ(
			//	targets.front().getTransform()(2, 3) + approachHeight
			//);
			//addTravelMove(module, proc, approachTarget, zone, tool, wobj);

			//// Move down to start position
			//addTravelMove(module, proc, targets.front(), zone, tool, wobj);

			//// Start printing
			//addPrintMove(module, proc, targets, zone, tool, wobj);

			//// Lift after printing
			//Target liftTarget = targets.back().withZ(
			//	targets.back().getTransform()(2, 3) + approachHeight
			//);
			//addTravelMove(module, proc, liftTarget, zone, tool, wobj);
		}

		void RapidMaker::addTravelMove(Module& module, Procedure& proc, const Target& target,
			const ZoneData* zone,
			const ToolData* tool,
			const WObjData* wobj) {
			const auto& settings = module.getSettings();
			proc.addSetDO(RapidSignals::START_EXTRUSION, false);
			proc.addSetAO(RapidSignals::EXTRUSION_RATE, 0);
			module.addMoveL(proc, target, settings.travelSpeed, wobj, tool, zone);
		}

		string RapidMaker::createFilePath(const string& baseFilePath, const string& moduleName) const {
			string path = baseFilePath;
			// Replace forward slashes with backslashes for Windows
			replace(path.begin(), path.end(), '/', '\\');

			// Ensure the directory exists
			size_t lastSlash = path.find_last_of('\\');
			if (lastSlash != string::npos) {
				string dir = path.substr(0, lastSlash);
				system(("mkdir \"" + dir + "\" 2>nul").c_str());
			}

			return path + "_" + moduleName + ".mod";
		}

		bool RapidMaker::generateRapidFiles(const string& baseFilePath) {
			bool success = true;
			int filesGenerated = 0;

			for (const auto& module : modules) {
				string filepath = createFilePath(baseFilePath, module.getName());
				if (module.saveToFile(filepath)) {
					cout << "Generated file: " << filepath << endl;
					filesGenerated++;
				}
				else {
					cerr << "Failed to generate file: " << filepath << endl;
					success = false;
				}
			}

			cout << "Total files generated: " << filesGenerated << " of " << modules.size() << endl;
			return success && (filesGenerated > 0);
		}
	}
}