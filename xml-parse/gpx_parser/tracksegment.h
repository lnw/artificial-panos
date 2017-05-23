#ifndef __TRACKSEGMENT_H__
#define __TRACKSEGMENT_H__

#ifndef PI
#define PI 3.141592654
#endif


class tracksegment{
//private:

	//the four internal types ...
	std::vector<double> latitudeList;
	std::vector<double> longitudeList;
	std::vector<double> elevationList;
	std::vector<long unsigned int> timeList;

	std::vector<double>* select_vector(std::string choice); //wir nennen es Notloesung

	void do_gaussian_smooth(std::vector<double>* chosen_list, float sigma);

public:
	//constructors
	//vielleicht brauch ich die gar nicht
//	track(): std::vector<double> latitudeList, std::vector<double> longitudeList, std::vector<double> heightList, std::vector<double> timeList {};

	tracksegment& append_point(double lat, double lon, double ele, long unsigned int time);

	std::vector<double>::const_iterator lat_begin() const;
	std::vector<double>::const_iterator lon_begin() const;
	std::vector<double>::const_iterator ele_begin() const;
	std::vector<long unsigned int>::const_iterator time_begin() const;

	std::vector<double>::const_iterator lat_end() const;
	std::vector<double>::const_iterator lon_end() const;
	std::vector<double>::const_iterator ele_end() const;
	std::vector<long unsigned int>::const_iterator time_end() const;

	double lat(std::vector<double>::const_iterator lat_iter) const;
	double lon(std::vector<double>::const_iterator lon_iter) const;
	double ele(std::vector<double>::const_iterator ele_iter) const;
	long unsigned int time(std::vector<long unsigned int>::const_iterator time_iter) const;

	int size() const;

	tracksegment& gaussian_smooth(std::string choice, float sigma);

	//entfernen der oscillation die entstehen wenn man stehen bleibt oder ein Gebäude betritt.
	//Fuer jeden Punkt werden die Punkte von <Punkt - range> bis <Punkt + range>
	//betrachtet.  Für jedes Paar P, P' wird der Quotient aus Wegstrecke und direkter
	//Entfernung zwischen P und P' gebildet.  Aus den je <range> Quotienten vor und hinter P wird der
	//Durchschnitt gebildet.  Liegen die Punkte auf einer gerade Linie, so ist jeder
	//Quotient 1, im Fall starken Oszillierens werden sie größer.  Ein Punkt sollte
	//nur dann geloescht werden, wenn zu beiden Seiten ein Grenzwert überschritten wird.
	tracksegment& remove_clutter(int range, float cutoff);

	double ascent() const;
	double descent() const;
	double lateral_distance() const;

	tracksegment& null_times();
	tracksegment& set_times(long unsigned int offset);
	
	tracksegment& crop(float area_lon_min, float area_lat_min, float area_lon_max, float area_lat_max);
};


double deg2rad(double deg);
double rad2deg(double rad);

//Entfernung zwischen zwei Punkten a und b entlang von Grosskreisen auf einer Kugel.
//lambda sind grad westlicher länge (longitude) und phi grad noerdlicher breite (latitude).
double great_circle_distance(double phi_a, double lambda_a, double phi_b, double lambda_b);

double gaussian(double x, float sigma);


#endif // __TRACKSEGMENT_H__
