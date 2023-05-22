#include "async/FileIOProcessor.hpp"
#include "async/status.hpp"
#include <iostream>

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cout << "Usage: " << argv[0] << " file_path file_content"
				  << std::endl;
		return (2);
	}
	try
	{
		async::FileWriter writer(0, argv[1], argv[2]);

		int rc;
		while (true)
		{
			rc = writer.task();
			if (rc == async::status::OK)
				break;
			else if (rc == async::status::AGAIN)
				continue;
			else
				throw(std::logic_error("Impossible return code"));
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}
	return (0);
}
