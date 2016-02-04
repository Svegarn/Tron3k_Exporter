#define NT_PLUGIN
#define REQUIRE_IOSTREAM
#define EXPORT __declspec(dllexport)

#include <maya/MFnPlugin.h>
#include "includes\DataHandler.h"

EXPORT MStatus initializePlugin(MObject obj){
	MStatus res;

	MFnPlugin plugin(obj, "Exporter", "1.0", "Any", &res);
	if (MFAIL(res)){
		CHECK_MSTATUS(res);
	}

	MGlobal::displayInfo("Exporter loaded...");

	res = plugin.registerCommand("DataHandler", DataHandler::creator);

	if (res != MS::kSuccess) {
		res.perror("Could not register command...");
		return res;
	}

	return res;
}

EXPORT MStatus uninitializePlugin(MObject obj){
	MStatus res;

	MFnPlugin plugin(obj);

	MGlobal::displayInfo("Exporter unloaded...");

	res = plugin.deregisterCommand("DataHandler");

	// check for an error
	if (res != MS::kSuccess) {

		res.perror("Could not de-register command...");
		return res;
	}

	return MS::kSuccess;
}