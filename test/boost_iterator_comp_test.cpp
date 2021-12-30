#include <boost/container/string.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>

int main()
{
    using String = boost::container::string;
    using Iterator = boost::archive::iterators::binary_from_base64<String::iterator>;

    String s;
    s.assign(Iterator{s.begin()},Iterator{s.end()});
    return 0;
}