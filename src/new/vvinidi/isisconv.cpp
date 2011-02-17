#include <DataStorage/io_application.hpp>
#include <DataStorage/io_factory.hpp>
#include <DataStorage/image.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <list>

extern "C" {
	void getLipsiaVersion(char*,size_t);
}

using namespace isis;

int main( int argc, char **argv )
{
	isis::util::enableLog<isis::util::DefaultMsgPrint>( isis::error );
	isis::data::enableLog<isis::util::DefaultMsgPrint>( isis::error );
	isis::image_io::enableLog<isis::util::DefaultMsgPrint>( isis::error );
	std::cout << "isis core version: " << isis::util::Application::getCoreVersion() << std::endl;
	char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vvinidi V%s", ver);
	fprintf(stderr, "%s\n", prg_name);
	data::IOApplication app( "isis data converter", true, true );
	app.parameters["tr"] = 0.;
	app.parameters["tr"].needed() = false;
	app.parameters["tr"].setDescription( "Repetition time in s" );
	app.init( argc, argv ); // will exit if there is a problem

	if( app.parameters["tr"]->as<double>() > 0 ) {
		BOOST_FOREACH( std::list<data::Image>::reference ref, app.images ) {
			ref.setPropertyAs<u_int16_t>( "repetitionTime", app.parameters["tr"]->as<double>() * 1000 );
		}
	}

	if( app.images.size() > 1 ) {
		//we have to sort the output images by the sequenceStart so the number in the output filename represents the
		//number in the scan protocol
		typedef std::multimap< boost::posix_time::ptime, data::Image > timeStampsType;
		timeStampsType timeStamps;
		BOOST_FOREACH( std::list<data::Image>::const_reference image, app.images ) {
			timeStamps.insert( std::make_pair< boost::posix_time::ptime, data::Image >(
								   image.getPropertyAs< boost::posix_time::ptime >( "sequenceStart" ), image ) ) ;
		}

		if ( timeStamps.size() != app.images.size() ) {
			LOG( ImageIoLog, warning ) << "Number of images and number of different timestamps do not coincide!"
									   << timeStamps.size() << " != " << app.images.size();
		}

		size_t count = 1;
		BOOST_FOREACH( timeStampsType::const_reference map, timeStamps ) {
			boost::filesystem::path out( app.parameters["out"].toString() );
			std::stringstream countString;
			countString << count++ << "_" << out.leaf();
			boost::filesystem::path newPath( out.branch_path() /  countString.str() );
			std::list<data::Image> tmpList;
			tmpList.push_back( map.second );
			data::IOFactory::write( tmpList, newPath.string(), app.parameters["wf"], app.parameters["wdialect"] );
		}
	} else {
		app.autowrite( app.images );
	}

	return EXIT_SUCCESS;
}
