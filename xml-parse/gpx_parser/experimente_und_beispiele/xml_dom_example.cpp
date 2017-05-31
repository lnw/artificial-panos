#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxml++/libxml++.h>

#include <iostream>

void print_indentation(unsigned int indentation)
{
  for(unsigned int i = 0; i < indentation; ++i)
    std::cout << " ";
}

//void print_node(const xmlpp::Node* node, unsigned int indentation = 0)
void print_node(const xmlpp::Node *node)
{
//  std::cout << std::endl; //Separate nodes by an empty line.
  
  const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  if(nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
    return;
    
  Glib::ustring nodename = node->get_name();

  if(!nodeText && !nodeComment && !nodename.empty()) //Let's not say "name: text". //textNodes und commentNodes haben als Namen "text" (which is not helpful)
  {
//    print_indentation(indentation);
//    std::cout << "Node name = " << node->get_name() << std::endl; //lnw: das ist doch das gleiche?
//    std::cout << "Node name = " << nodename << std::endl;
	if(nodename.compare("ele") == 0)
	{
    	xmlpp::Node::NodeList ele_list = node->get_children();
		xmlpp::TextNode* myNode = dynamic_cast<xmlpp::TextNode*>(*ele_list.begin()); // es gibt nur ein element
		std::cout << "elevation: " << myNode->get_content() << std::endl;
	}
	if(nodename.compare("time") == 0)
	{
    	xmlpp::Node::NodeList ele_list = node->get_children();
		xmlpp::TextNode* myNode = dynamic_cast<xmlpp::TextNode*>(*ele_list.begin()); // es gibt nur ein element
		std::cout << "time: " << myNode->get_content() << std::endl;
	}
  }
//  else if(nodeText) //Let's say when it's text. - e.g. let's say what that white space is.
//  {
//    print_indentation(indentation);
//    std::cout << "Text Node [nodeText] " << std::endl;
//  }

  //Treat the various node types differently: 
  if(nodeText)
  {
//    print_indentation(indentation);
//    std::cout << "text = \"" << nodeText->get_content() << "\"" << std::endl;
  }
//  else if(nodeComment)
//  {
//    print_indentation(indentation);
//    std::cout << "comment = " << nodeComment->get_content() << std::endl;
//  }
//  else if(nodeContent) //nodeContent ist die Uebermenge von nodeComment, nodeText und anderen
//  {
//    print_indentation(indentation);
//    std::cout << "content = " << nodeContent->get_content() << std::endl;
//  }
  else if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node))
  {
    //A normal Element node:

    //line() works only for ElementNodes.
//    print_indentation(indentation);
//    std::cout << "line = " << node->get_line() << std::endl;

    //Print attributes:
//    const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
//    for(xmlpp::Element::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
//    {
//      const xmlpp::Attribute* attribute = *iter;
//      print_indentation(indentation);
//      std::cout << "Attribute " << attribute->get_name() << " = " << attribute->get_value() << std::endl;
//    }

    const xmlpp::Attribute* lat_attribute = nodeElement->get_attribute("lat");
    if(lat_attribute)
    {
      std::cout << "latitude found: " << lat_attribute->get_value() << std::endl;
      
    }
    const xmlpp::Attribute* lon_attribute = nodeElement->get_attribute("lon");
    if(lon_attribute)
    {
      std::cout << "longitude found: " << lon_attribute->get_value() << std::endl;
      
    }
  }
  
  if(!nodeContent)
  {
    //Recurse through child nodes:
    xmlpp::Node::NodeList list = node->get_children();
    for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
    {
//      print_node(*iter, indentation + 2); //recursive
		print_node(*iter);
    }
  }
}

int main(int argc, char* argv[])
{
  std::string filepath;
  if(argc > 1 )
    filepath = argv[1]; //Allow the user to specify a different XML file to parse.
  else
    filepath = "example.xml";
  
  try
  {
    xmlpp::DomParser parser;
//    parser.set_validate();
//    parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
    parser.parse_file(filepath);
    if(parser)
    {
      //Walk the tree:
      const xmlpp::Node* pNode = parser.get_document()->get_root_node(); //deleted by DomParser.
      print_node(pNode);
    }
  }
  catch(const std::exception& ex)
  {
    std::cout << "Exception caught: " << ex.what() << std::endl;
  }

  return 0;
}
