/* vim: set tabstop=4 : */
#include <febird/io/IStream.h>

namespace febird {

stream_position_t ISeekable::size()
{
	stream_position_t old_pos = tell();
	seek(0, 2); // seek to end
	stream_position_t nsize = tell();
	seek(old_pos);
	return nsize;
}

void ISeekable::seek(stream_position_t position)
{
	seek(stream_offset_t(position), 0);
}

IAcceptor::~IAcceptor()
{

}

} // namespace febird
