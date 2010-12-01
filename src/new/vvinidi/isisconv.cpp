#include <DataStorage/io_application.hpp>
#include <DataStorage/io_factory.hpp>
#include <DataStorage/image.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <list>


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
	if( app.images.size() > 1 ) {
		typedef std::multimap< boost::posix_time::ptime, boost::shared_ptr<data::Image> > timeStampsType;
		timeStampsType timeStamps;
		BOOST_FOREACH( std::list<boost::shared_ptr<data::Image> >::const_reference image, app.images ) 
		{
			timeStamps.insert( std::make_pair< boost::posix_time::ptime, boost::shared_ptr< data::Image > >(
				image->getProperty< boost::posix_time::ptime >("sequenceStart"), image ) ) ;
		}
		if ( timeStamps.size() != app.images.size() ) 
		{
			LOG( ImageIoLog, warning ) << "Number of images and number of different timestamps do not coincide!"
											 << timeStamps.size() << " != " << app.images.size();
		}
		size_t count = 1;
		BOOST_FOREACH( timeStampsType::const_reference map, timeStamps )
		{
			boost::filesystem::path out( app.parameters["out"].toString() );
			std::stringstream newFileName;
			newFileName << out.branch_path() << "/S" << count++ << "_" << out.leaf();
			data::ImageList tmpList;
			tmpList.push_back( map.second );
			data::IOFactory::write(tmpList, newFileName.str(), "", "");
		}
	} else {
		app.autowrite( app.images );
	}
	
	return EXIT_SUCCESS;
}
