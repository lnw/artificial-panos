Glib::ustring myASCIIdata("123456");
std::stringstream s;
long result;

s << myASCIIdata.raw();
s >> result;
