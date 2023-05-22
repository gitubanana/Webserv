#include "async/FileIOProcessor.hpp"
#include "async/status.hpp"

using namespace async;

FileWriter::FileWriter() : FileIOProcessor()
{
}

FileWriter::FileWriter(const FileWriter &orig) : FileIOProcessor(orig)
{
}

FileWriter &FileWriter::operator=(const FileWriter &orig)
{
	FileIOProcessor::operator=(orig);
	return (*this);
}

FileWriter::FileWriter(unsigned int timeout_ms, int fd,
					   const std::string &content)
	: FileIOProcessor(timeout_ms, fd), _content(content)
{
}

FileWriter::FileWriter(unsigned int timeout_ms, const std::string &path,
					   const std::string &content)
	: FileIOProcessor(timeout_ms, path), _content(content)
{
}

FileWriter::~FileWriter()
{
}

int FileWriter::task(void)
{
	if (_status == status::OK)
		return (_status);
	if (_status == status::BEGIN)
	{
		openFdByPath();
		_processor = new SingleIOProcessor(_fd);
		_processor->setWriteBuf(_content);
		_status = status::AGAIN;
	}

	checkTimeout();
	_processor->task();
	if (_processor->writeDone())
		_status = status::OK;
	return (_status);
}
