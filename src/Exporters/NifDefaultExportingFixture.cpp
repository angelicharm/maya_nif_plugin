#include "include/Exporters/NifDefaultExportingFixture.h"

NifDefaultExportingFixture::NifDefaultExportingFixture() {

}

NifDefaultExportingFixture::NifDefaultExportingFixture( NifTranslatorDataRef translatorData, NifTranslatorOptionsRef translatorOptions,
	NifTranslatorUtilsRef translatorUtils, NifNodeExporterRef nodeExporter, NifMeshExporterRef meshExporter, NifMaterialExporterRef materialExporter, NifAnimationExporterRef animationExporter) {
		this->translatorOptions = translatorOptions;
		this->translatorData = translatorData;
		this->translatorUtils = translatorUtils;
		this->nodeExporter = nodeExporter;
		this->meshExporter = meshExporter;
		this->materialExporter = materialExporter;
		this->animationExporter = animationExporter;
}

MStatus NifDefaultExportingFixture::WriteNodes( const MFileObject& file ) {
	try {
	//out << "Creating root node...";
	//Create new root node
	this->translatorData->sceneRoot = new NiNode;
	this->translatorData->sceneRoot->SetName( "Scene Root" );
	//out << sceneRoot << endl;

	//out << "Exporting file textures..." << endl;
	this->translatorData->textures.clear();
	//Export file textures and get back a map of DAG path to Nif block
	this->materialExporter->ExportFileTextures();


	//out << "Exporting shaders..." << endl;
	this->translatorData->shaders.clear();
	//Export shaders
	this->materialExporter->ExportShaders();


	//out << "Clearing Nodes" << endl;
	this->translatorData->nodes.clear();
	//out << "Clearing Meshes" << endl;
	this->translatorData->meshes.clear();
	//Export nodes

	//out << "Exporting nodes..." << endl;
	this->nodeExporter->ExportDAGNodes();

	//out << "Enumerating skin clusters..." << endl;
	this->translatorData->meshClusters.clear();
	this->meshExporter->EnumerateSkinClusters();

	//out << "Exporting meshes..." << endl;
	for ( list<MObject>::iterator mesh = this->translatorData->meshes.begin(); mesh != this->translatorData->meshes.end(); ++mesh ) {
		this->meshExporter->ExportMesh( *mesh );
	}

	//out << "Applying Skin offsets..." << endl;
	this->meshExporter->ApplyAllSkinOffsets( StaticCast<NiAVObject>(this->translatorData->sceneRoot) );



	//--Write finished NIF file--//

	//out << "Writing Finished NIF file..." << endl;
	NifInfo nif_info(this->translatorOptions->export_version, this->translatorOptions->export_user_version);
	nif_info.endian = ENDIAN_LITTLE; //Intel endian format
	nif_info.exportInfo1 = "NifTools Maya NIF Plug-in " + string(PLUGIN_VERSION);
	WriteNifTree( file.fullName().asChar(), StaticCast<NiObject>(this->translatorData->sceneRoot), nif_info );

	//out << "Export Complete." << endl;

	//Clear temporary data
	//out << "Clearing temporary data" << endl;
	this->translatorData->Reset();
	}
	catch( exception & e ) {
		stringstream out;
		out << "Error:  " << e.what() << endl;
		MGlobal::displayError( out.str().c_str() );
		return MStatus::kFailure;
	}
	catch( ... ) {
		MGlobal::displayError( "Error:  Unknown Exception." );
		return MStatus::kFailure;
	}

	#ifndef _DEBUG
	//Clear the stringstream so it doesn't waste a bunch of RAM
	out.clear();
	#endif

	return MS::kSuccess;
}

string NifDefaultExportingFixture::asString( bool verbose /*= false */ ) const {
	stringstream out;

	out<<NifExportingFixture::asString(verbose)<<endl;
	out<<this->nodeExporter->asString(verbose)<<endl;
	out<<this->meshExporter->asString(verbose)<<endl;
	out<<this->materialExporter->asString(verbose)<<endl;

	return out.str();
}

const Type& NifDefaultExportingFixture::getType() const {
	return TYPE;
}


const Type NifDefaultExportingFixture::TYPE("NifDefaultExportingFixture",&NifExportingFixture::TYPE);