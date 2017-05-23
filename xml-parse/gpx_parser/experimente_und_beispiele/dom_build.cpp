// -*- C++ -*-

/* main.cc
 *
 * Copyright (C) 2002 The libxml++ development team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxml++/libxml++.h>

#include <iostream>


int main(int /* argc */, char** /* argv */)
{
    xmlpp::Document document_object;
    xmlpp::Document *document = &document_object;
    xmlpp::Element* nodeRoot = document->create_root_node("exampleroot", "", ""); //Declares the namespace and uses its prefix for this node
    xmlpp::Element* nodeChild = nodeRoot->add_child("examplechild");
////man nennt einfach *alle* knoten die nciht root sind nodeChild und sogar diese Unterscheidung liesse sich einsparen.
//    nodeChild->set_namespace("bar"); //So it will be bar::examplechild.
    nodeChild->set_attribute("id", "1", ""); //foo is the namespace prefix. You could also just use a name of foo:id".
    nodeChild->set_child_text("Some content");
//    nodeChild->add_child_comment("Some comments");
    nodeChild = nodeChild->add_child("child_of_child", "");
	nodeChild->set_child_text("foo");

    nodeChild = nodeRoot->add_child("examplechild", ""); //foobar is the namespace prefix
    nodeChild->set_attribute("id", "2", ""); //foobar is the namespace prefix.

    Glib::ustring whole = document->write_to_string();
    std::cout << "XML built at runtime: " << std::endl << whole << std::endl;
//    std::cout << "default namespace: " << nodeRoot->get_namespace_uri() << std::endl;
	document->write_to_file_formatted("foo.xml");

  return 0;
}

