#include <DataStorage/io_application.hpp>
#include <DataStorage/io_factory.hpp>
#include <boost/foreach.hpp>


using namespace isis;

int main( int argc, char **argv )
{
    isis::util::enable_log<isis::util::DefaultMsgPrint>(isis::error);
    isis::data::enable_log<isis::util::DefaultMsgPrint>(isis::error);
    isis::image_io::enable_log<isis::util::DefaultMsgPrint>(isis::error);
        
	data::IOApplication app( "isis data converter", true, true );
	app.parameters["tr"] = 0.;
	app.parameters["tr"].needed() = false;
	app.parameters["tr"].setDescription( "Repetition time in s" );
	app.init( argc, argv ); // will exit if there is a problem

	if( app.parameters["tr"]->as<double>() > 0 ) {
		BOOST_FOREACH( data::ImageList::const_reference ref, app.images ) {
			ref->setProperty<u_int16_t>( "repetitionTime", app.parameters["tr"]->as<double>() * 1000 );
		}
	}

	app.autowrite( app.images );
	return EXIT_SUCCESS;
}
