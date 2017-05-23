
#include <cstdlib>
#include <vector>
#include <math.h>
#include <iostream>
#include <iomanip> //required for setfill()
#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <libxml++/libxml++.h>

#include "track.h"
#include "tracksegment.h"
#include "conversions.h"

//using namespace std;

namespace po = boost::program_options;



//parst das ganze xml-objekt, hängt punkte an das *letzte* tracksegment an und beginnt gegebenenfalls vorher ein neues tracksegment
void parse_gpx(const xmlpp::Node *node, track &track, float lateral_cutoff, float time_cutoff)
{
	const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
	const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
	const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

	if(nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
	{
		return;
    }

	Glib::ustring nodename = node->get_name();

	if(nodename.compare("trkpt") == 0)
	{
		const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
		//lat und lon stehen in attributen
		double lat = to_double(nodeElement->get_attribute("lat")->get_value());
		double lon = to_double(nodeElement->get_attribute("lon")->get_value());
		//ele und time in childnodes
		double ele;
		long unsigned int time;
		xmlpp::Node::NodeList list = node->get_children();
		for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
		{
			if((*iter)->get_name().compare("ele") == 0) //compare ist 0, wenn true
			{
    			xmlpp::Node::NodeList ele_list = (*iter)->get_children();
				xmlpp::TextNode* myNode = dynamic_cast<xmlpp::TextNode*>(*ele_list.begin()); // es gibt nur ein element; cast von Node nach TextNode
				ele = to_double(myNode->get_content());
				//std::cout << "ele: " << ele << std::endl;
			}
			if((*iter)->get_name().compare("time") == 0)
			{
    			xmlpp::Node::NodeList time_list = (*iter)->get_children();
				xmlpp::TextNode* myNode = dynamic_cast<xmlpp::TextNode*>(*time_list.begin()); // es gibt nur ein element; cast von Node nach TextNode
				time = ustring_to_sec(myNode->get_content());
				//std::cout << "time: " << time << std::endl;
			}
		}
		//testen ob ein neues tracksegment begonnen werden soll
		if(track.back().lat_begin() != track.back().lat_end())//testen ob schon mindestens ein punkt im letzten tracksegment ist (nur notwendig fürs erste tracksegment)
		{
			const double lateral_diff = 1000*great_circle_distance(*(track.back().lat_end()-1), *(track.back().lon_end()-1), lat, lon);
			const double abs_lateral_diff = lateral_diff > 0 ? lateral_diff : -lateral_diff;
			const int time_diff = *(track.back().time_end()-1) - time;
			const unsigned int abs_time_diff = time_diff > 0 ? time_diff : -time_diff;
			//std::cout << track.back().size() << "\t" << abs_lateral_diff << "\t" << abs_time_diff << std::endl;
			if(abs_lateral_diff > lateral_cutoff || abs_time_diff > time_cutoff)//testen ob sich Ort oder Zeit stark geaendert haben
			{
				track.append_segment();
			}
		}
		//hinzufuegen des Punktes an das Ende des letzten tracksegments
		track.back().append_point(lat, lon, ele, time);
		return; //und da es unterhalb von trkpt keine weiteren Punkte gibt, braucht man hier nicht weiter parsen
	}
  
	if(!nodeContent)
	{
		//Recurse through child nodes:
		xmlpp::Node::NodeList list = node->get_children();
		for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
		{
			parse_gpx(*iter, track, lateral_cutoff, time_cutoff);
		}
	}
}


void write_gpx(track &track, std::string out_filename)
{
	xmlpp::Document out_document_object; //the root object
	xmlpp::Document *out_document = &out_document_object; // link to the root object (macht den code schoener)
	xmlpp::Element *gpx = out_document->create_root_node("gpx", "", "");
	xmlpp::Element *trk = gpx->add_child("trk");

	//iterieren ueber die segmente
	std::vector<tracksegment>::const_iterator trackSegIter = track.begin();
	while(trackSegIter < track.end())
	{
		xmlpp::Element *trkseg = trk->add_child("trkseg");
		//iterieren ueber die punkte
		std::vector<double>::const_iterator latIter = trackSegIter->lat_begin();
		std::vector<double>::const_iterator lonIter = trackSegIter->lon_begin();
		std::vector<double>::const_iterator eleIter = trackSegIter->ele_begin();
		std::vector<unsigned long int>::const_iterator tIter = trackSegIter->time_begin();
		while(latIter < trackSegIter->lat_end())
		{
			xmlpp::Element *trkpt = trkseg->add_child("trkpt");
			trkpt->set_attribute("lat", double_to_ustring(trackSegIter->lat(latIter)), "");
			trkpt->set_attribute("lon", double_to_ustring(trackSegIter->lon(lonIter)), "");
			xmlpp::Element *ele = trkpt->add_child("ele");
			ele->set_child_text(double_to_ustring(trackSegIter->ele(eleIter)));
			xmlpp::Element *time = trkpt->add_child("time");
			time->set_child_text(sec_to_ustring(trackSegIter->time(tIter)));
			++latIter;
			++lonIter;
			++eleIter;
			++tIter;
		}
		++trackSegIter;
	}
	//rausschreiben
	out_document->write_to_file_formatted(out_filename);	
	std::cout << "Written " << track.size() << " tracksegments with a total of ";
	int total_point_number = 0;
	for(std::vector<tracksegment>::const_iterator trackSegIter = track.begin(); trackSegIter < track.end(); ++trackSegIter)
	{
		total_point_number += trackSegIter->size();
	}
	std::cout << total_point_number << " points." << std::endl; 
}

// stolen from /usr/share/doc/libboost1.42-doc/examples/libs/program_options/example/real.cpp
// Function used to check that 'opt1' and 'opt2' are not specified at the same time.
void conflicting_options(const po::variables_map& vm, const char* opt1, const char* opt2)
{
    if (vm.count(opt1) && !vm[opt1].defaulted() && vm.count(opt2) && !vm[opt2].defaulted())
	{
		throw std::logic_error(std::string("Conflicting options '") + opt1 + "' and '" + opt2 + "'.");
	}
}


int main(int argc, char* argv[])
{
	// Declare the supported options.
	po::options_description desc("Allowed options", 110, 40); //gesamte Spaltenbreite, erste Spaltenbreite

	std::string in_filename;
	std::string out_filename;
	float xy_sigma;
	bool xy_gauss = 0;
	float z_sigma;
	bool z_gauss = 0;
	int rem_clutter_range;
	float rem_clutter_cutoff;
	bool rem_clutter = 0;
	int short_seg_cutoff;
	bool rem_short_seg = 0;
	float split_dist_cutoff;
	long unsigned int split_time_cutoff;
	bool null_times = 0;
	long unsigned int first_time;
	bool shift_times = 0;
	float area_lon_min;
	float area_lat_min;
	float area_lon_max;
	float area_lat_max;
	std::string area;
	bool area_crop = 0;

	po::positional_options_description pos_opt;
	pos_opt.add("input-file", -1);

	desc.add_options()
	    ("help,h", "display help message")
		("input-file,i", po::value<std::string>(&in_filename)->required(), "input file name")
		("output-file,o", po::value<std::string>(&out_filename), "output file name")
		("xy_gauss,x", po::value<float>(&xy_sigma)->implicit_value(1.5), "Perform gaussian smooth on lat and long: choose sigma")
		("z_gauss,z", po::value<float>(&z_sigma)->implicit_value(3.0), "Perform gaussian smooth on elevation data: choose sigma")
		("rem-clutter_range,r", po::value<int>(&rem_clutter_range)->implicit_value(3), "Remove points that mostly oscillate around some coordinate: choose evaluated range")
		("rem-clutter_cutoff,c", po::value<float>(&rem_clutter_cutoff)->implicit_value(1.05), "Remove points that mostly oscillate around some coordinate: choose cutoff")
		("rem-short-segments,s", po::value<int>(&short_seg_cutoff)->implicit_value(5), "Remove track segments that do not exceed a certain length: choose cutoff")
		("track-split_dist_cutoff,d", po::value<float>(&split_dist_cutoff)->default_value(500), "Split track into track segments if distance between adjacent points exceeds given length (in m)")
		("track-split_time_cutoff,t", po::value<long unsigned int>(&split_time_cutoff)->default_value(300), "Split track into track segments if time between adjacent points exceeds given length of time (in s)")
		("null_times,0", po::value<bool>(&null_times)->zero_tokens(), "Set the time of every point to 0 (sec since 1970) [might be useful for privacy reasons]")
		("shift_times_to,1", po::value<long unsigned int>(&first_time)->implicit_value(0), "Set time of first point to given value (in sec since 1970), but keep time offsets [might be useful for privacy reasons]")
		("crop_area,a", po::value<std::string>(&area), "Delete all points outside a given area: <lon_min>:<lat_min>:<lon_max>:<lat_max>")
	;

	po::variables_map vm;
	//po::store(po::parse_command_line(argc, argv, desc), vm); //simple
	po::store(po::command_line_parser(argc, argv).options(desc).positional(pos_opt).run(), vm); //more sophisticated
	
	conflicting_options(vm, "null_times", "shift_times_to");

	if(vm.count("help"))
	{
	    std::cout << desc << std::endl;
	    return 0;
	}

	po::notify(vm); //fist the "help", then notify because otherwise the ".required()"-option fails for any other parameter 

	std::cout << "Parsing command line ..." << std::endl;

	if(vm.count("input-file"))
	{
		//in_filename = vm["input-file"].as<std::string>(); //implizit
		std::cout << "input name: " << in_filename << std::endl;
	}

	if(vm.count("output-file"))
	{
		std::cout << "output name: " << out_filename << std::endl;
	}
	else
	{
		boost::regex re("(.*?)/?([^/]+)\\.(\\w+)");
		boost::cmatch matches;
 		if(boost::regex_match(in_filename.c_str(), matches, re))
		{
			std::string out_file_path(matches[1].first, matches[1].second);
			std::string out_file_body(matches[2].first, matches[2].second);
			std::string out_file_extension(matches[3].first, matches[3].second);
			if(out_file_path.compare("") != 0) //if not empty
			{
				out_filename = out_file_path + "/" + out_file_body + "_refined." + out_file_extension;
			}
			else
			{
				out_filename = out_file_body + "_refined." + out_file_extension;
			}
			std::cout << "output name: " << out_filename << std::endl;
		}
		else
		{
			std::cout << "Unfortunately there was an error parsing the in_filename and generating an out_filename.  That's probably a programming error." << std::endl;
			return 1;
		}
	}

	if(vm.count("xy_gauss"))
	{
		xy_gauss = 1;
		std::cout << "The lists of latitudes and longitudes will be convoluted with a gaussian (\\sigma = " << xy_sigma << ")." << std::endl;
	} 

	if(vm.count("z_gauss"))
	{
		z_gauss = 1;
		std::cout << "The list of elevations will be convoluted with a gaussian (\\sigma = " << z_sigma << ")." << std::endl;
	} 

	//die folgenden vier Ausdruecke sind haesslich, aber es geht so
	if(vm.count("rem-clutter_range") && vm.count("rem-clutter_cutoff"))
	{
		rem_clutter = 1;
	}

	if(vm.count("rem-clutter_range") && !vm.count("rem-clutter_cutoff"))
	{
		rem_clutter = 1;
		rem_clutter_cutoff = 1.05;
	}

	if(!vm.count("rem-clutter_range") && vm.count("rem-clutter_cutoff"))
	{
		rem_clutter = 1;
		rem_clutter_range = 3;
	}

	if(vm.count("rem-clutter_range") || vm.count("rem-clutter_cutoff"))
	{
		std::cout << "remove clutter: range = " << rem_clutter_range << std::endl;
		std::cout << "remove clutter: cutoff = " << rem_clutter_cutoff << std::endl;
	}

	if(vm.count("rem-short-segments"))
	{
		rem_short_seg = 1;
		std::cout << "remove short segments: cutoff = " << short_seg_cutoff << std::endl;
	}

	if(vm.count("null_times"))
	{
		std::cout << "set time of all points to 0s (since 1970)" << std::endl;
	}

	if(vm.count("shift_times_to"))
	{
		shift_times = 1;
		std::cout << "set time of first point to " << first_time << "s (since 1970) and shift all other times accordingly" << std::endl;
	}

	if(vm.count("crop_area"))
	{
		area_crop = 1;
		boost::regex re("(-?\\d*\\.?\\d*):(-?\\d*\\.?\\d*):(-?\\d*\\.?\\d*):(-?\\d*\\.?\\d*)");
		boost::cmatch matches;
 		if(boost::regex_match(area.c_str(), matches, re))
		{
			std::string area_lon_min_s(matches[1].first, matches[1].second);
			std::string area_lat_min_s(matches[2].first, matches[2].second);
			std::string area_lon_max_s(matches[3].first, matches[3].second);
			std::string area_lat_max_s(matches[4].first, matches[4].second);
			area_lon_min = to_double(area_lon_min_s);
			area_lat_min = to_double(area_lat_min_s);
			area_lon_max = to_double(area_lon_max_s);
			area_lat_max = to_double(area_lat_max_s);
		}
		else
		{
			std::cout << "Invalid area string given.  Please try again." << std::endl;
			return 1;
		}
		if(area_lon_min >= area_lon_max || area_lat_min >= area_lat_max)
		{
			std::cout << "Lower bounds should be lower than upper bounds." << std::endl;
			return 1;
		}
		std::cout << "Crop area: " << area_lon_min << " : " << area_lat_min << " : " << area_lon_max << " : " << area_lat_max <<  std::endl;
	}

//	std::cout.precision(10);


	std::cout << std::endl << "Parsing gpx ..." << std::endl;
	track track_name;
	track_name.append_segment();
	
	try
	{
		xmlpp::DomParser parser;
		parser.parse_file(in_filename);
		if(parser)
		{
			//find root node
			const xmlpp::Node* pNode = parser.get_document()->get_root_node();
			//print recursively
			parse_gpx(pNode, track_name, split_dist_cutoff, split_time_cutoff);
		}
	}
	catch(const std::exception& ex)
	{
		std::cout << "Exception caught: " << ex.what() << std::endl;
		return 1;
	}


	std::cout << std::endl << "Processing data (optional) ..." << std::endl;

	if(null_times)
	{
		track_name.null_times();
	}

	if(shift_times)
	{
		track_name.set_times(first_time);
	}

	if(area_crop)
	{
		track_name.crop(area_lon_min, area_lat_min, area_lon_max, area_lat_max);
	}

	if(rem_clutter)
	{
		track_name.remove_clutter(rem_clutter_range, rem_clutter_cutoff);
	}

	if(xy_gauss)
	{
		track_name.gaussian_smooth("xy", xy_sigma);
	}

	if(z_gauss)
	{
		track_name.gaussian_smooth("z", z_sigma);
	}

	if(rem_short_seg)
	{
		track_name.remove_short_segs(short_seg_cutoff);
	}

	std::cout << std::endl << "Additional info ..." << std::endl;

	std::cout << "lat_dist " << track_name.lateral_distance() << " km" << std::endl;
	std::cout << "ascent " << track_name.ascent() << " m" << std::endl;
	std::cout << "descent " << track_name.descent() << " m" << std::endl;


	std::cout << std::endl << "Writing ..." << std::endl;
	write_gpx(track_name, out_filename);

	return 0;
}

