#ifndef OSG2_BINARYSTREAMOPERATOR
#define OSG2_BINARYSTREAMOPERATOR

#include <osgDB/StreamOperator>
#include <osg/Types>
#include <vector>


class BinaryOutputIterator : public osgDB::OutputIterator
{
public:
    BinaryOutputIterator( std::ostream* ostream ) { _out = ostream; }
    virtual ~BinaryOutputIterator() {}

    virtual bool isBinary() const { return true; }

    virtual void writeBool( bool b )
    {
#ifdef IM_SIZE_REDUCTION
#else
      char c = b?1:0; _out->write( &c, osgDB::CHAR_SIZE );
#endif
    }

    virtual void writeChar( char c )
    {
#ifdef IM_SIZE_REDUCTION
#else
      _out->write( &c, osgDB::CHAR_SIZE );
#endif
    }

    virtual void writeUChar( unsigned char c )
    {
#ifdef IM_SIZE_REDUCTION
#else
      _out->write( (char*)&c, osgDB::CHAR_SIZE );
#endif
    }

    virtual void writeShort( short s )
    {
#ifdef IM_SIZE_REDUCTION
#else
      _out->write( (char*)&s, osgDB::SHORT_SIZE );
#endif
    }

    virtual void writeUShort( unsigned short s )
    {
#ifdef IM_SIZE_REDUCTION
#else
      _out->write( (char*)&s, osgDB::SHORT_SIZE );
#endif
    }

    virtual void writeInt( int i )
    {
#ifdef IM_SIZE_REDUCTION
#else
      _out->write( (char*)&i, osgDB::INT_SIZE );
#endif
    }

    virtual void writeUInt( unsigned int i )
    {
#ifdef IM_SIZE_REDUCTION
#else
      _out->write( (char*)&i, osgDB::INT_SIZE );
#endif
    }

    virtual void writeLong( long l )
    {
#ifdef IM_SIZE_REDUCTION
#else
        // On 64-bit systems a long may not be the same size as the file value
        int32_t value=(int32_t)l;
        _out->write( (char*)&value, osgDB::LONG_SIZE );
#endif
    }

    virtual void writeULong( unsigned long l )
    {
#ifdef IM_SIZE_REDUCTION
#else
        // On 64-bit systems a long may not be the same size as the file value
        uint32_t value=(int32_t)l;
        _out->write( (char*)&value, osgDB::LONG_SIZE );
#endif
    }

    virtual void writeFloat( float f )
    {
#ifdef IM_SIZE_REDUCTION
#else
      _out->write( (char*)&f, osgDB::FLOAT_SIZE );
#endif
    }

    virtual void writeDouble( double d )
    {
#ifdef IM_SIZE_REDUCTION
#else
      _out->write((char*)&d, osgDB::DOUBLE_SIZE);
#endif
    }

    virtual void writeString( const std::string& s )
    {
#ifdef IM_SIZE_REDUCTION
#else
        int size = s.size();
        _out->write( (char*)&size, osgDB::INT_SIZE );
        _out->write( s.c_str(), s.size() );
#endif
    }

    virtual void writeStream( std::ostream& (* /*fn*/)(std::ostream&) ) {}

    virtual void writeBase( std::ios_base& (* /*fn*/)(std::ios_base&) ) {}

    virtual void writeGLenum( const osgDB::ObjectGLenum& value )
    {
#ifdef IM_SIZE_REDUCTION
#else
      GLenum e = value.get(); _out->write((char*)&e, osgDB::GLENUM_SIZE);
#endif
    }

    virtual void writeProperty( const osgDB::ObjectProperty& prop )
    {
#ifdef IM_SIZE_REDUCTION
#else
      if (prop._mapProperty) _out->write((char*)&(prop._value), osgDB::INT_SIZE);
#endif
    }

    virtual void writeMark( const osgDB::ObjectMark& mark )
    {
#ifdef IM_SIZE_REDUCTION
#else
        if ( _supportBinaryBrackets )
        {
            if ( mark._name=="{" )
            {
                int size = 0;
                _beginPositions.push_back( _out->tellp() );
                _out->write( (char*)&size, osgDB::INT_SIZE );
            }
            else if ( mark._name=="}" && _beginPositions.size()>0 )
            {
                std::streampos pos = _out->tellp(), beginPos = _beginPositions.back();
                _beginPositions.pop_back();
                _out->seekp( beginPos );

                std::streampos size64 = pos - beginPos;
                int size = (int) size64;
                _out->write( (char*)&size, osgDB::INT_SIZE );
                _out->seekp( pos );
            }
        }
#endif
    }

    virtual void writeCharArray( const char* s, unsigned int size )
    {
#ifdef IM_SIZE_REDUCTION
#else
      if ( size>0 ) _out->write( s, size );
#endif
    }

    virtual void writeWrappedString( const std::string& str )
    {
#ifdef IM_SIZE_REDUCTION
#else
      writeString( str );
#endif
    }

protected:
    std::vector<std::streampos> _beginPositions;
};

class BinaryInputIterator : public osgDB::InputIterator
{
public:
    BinaryInputIterator( std::istream* istream, int byteSwap )
    {
        _in = istream;
        setByteSwap(byteSwap);
    }

    virtual ~BinaryInputIterator() {}

    virtual bool isBinary() const { return true; }

    virtual void readBool( bool& b )
    {
        char c = 0;
        _in->read( &c, osgDB::CHAR_SIZE );
        b = (c!=0);
    }

    virtual void readChar( char& c )
    { _in->read( &c, osgDB::CHAR_SIZE ); }

    virtual void readSChar( signed char& c )
    { _in->read( (char*)&c, osgDB::CHAR_SIZE ); }

    virtual void readUChar( unsigned char& c )
    { _in->read( (char*)&c, osgDB::CHAR_SIZE ); }

    virtual void readShort( short& s )
    {
        _in->read( (char*)&s, osgDB::SHORT_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&s, osgDB::SHORT_SIZE );
    }

    virtual void readUShort( unsigned short& s )
    {
        _in->read( (char*)&s, osgDB::SHORT_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&s, osgDB::SHORT_SIZE );
    }

    virtual void readInt( int& i )
    {
        _in->read( (char*)&i, osgDB::INT_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&i, osgDB::INT_SIZE );
    }

    virtual void readUInt( unsigned int& i )
    {
        _in->read( (char*)&i, osgDB::INT_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&i, osgDB::INT_SIZE );
    }

    virtual void readLong( long& l )
    {
        // On 64-bit systems a long may not be the same size as the file value
        int32_t value;
        _in->read( (char*)&value, osgDB::LONG_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&value, osgDB::LONG_SIZE );
        l = (long)value;
    }

    virtual void readULong( unsigned long& l )
    {
        uint32_t value;
        _in->read( (char*)&value, osgDB::LONG_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&value, osgDB::LONG_SIZE );
        l = (unsigned long)value;
    }

    virtual void readFloat( float& f )
    {
        _in->read( (char*)&f, osgDB::FLOAT_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&f, osgDB::FLOAT_SIZE );
    }

    virtual void readDouble( double& d )
    {
        _in->read( (char*)&d, osgDB::DOUBLE_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&d, osgDB::DOUBLE_SIZE );
    }

    virtual void readString( std::string& s )
    {
        int size = 0;
        readInt( size );
        if ( size>0 )
        {
            s.resize( size );
            _in->read( (char*)s.c_str(), size );
        }
        else if ( size<0 )
        {
            throwException( "InputStream::readString() error, negative string size read." );
        }
    }

    virtual void readStream( std::istream& (* /*fn*/)(std::istream&) ) {}

    virtual void readBase( std::ios_base& (* /*fn*/)(std::ios_base&) ) {}

    virtual void readGLenum( osgDB::ObjectGLenum& value )
    {
        GLenum e = 0;
        _in->read( (char*)&e, osgDB::GLENUM_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&e, osgDB::GLENUM_SIZE );
        value.set( e );
    }

    virtual void readProperty( osgDB::ObjectProperty& prop )
    {
        int value = 0;
        if ( prop._mapProperty )
        {
            _in->read( (char*)&value, osgDB::INT_SIZE );
            if ( _byteSwap ) osg::swapBytes( (char*)&value, osgDB::INT_SIZE );
        }
        prop.set( value );
    }

    virtual void readMark( osgDB::ObjectMark& mark )
    {
        if ( _supportBinaryBrackets )
        {
            if ( mark._name=="{" )
            {
                int size = 0;
                _beginPositions.push_back( _in->tellg() );

                _in->read( (char*)&size, osgDB::INT_SIZE );
                if ( _byteSwap ) osg::swapBytes( (char*)&size, osgDB::INT_SIZE );
                _blockSizes.push_back( size );
            }
            else if ( mark._name=="}" && _beginPositions.size()>0 )
            {
                _beginPositions.pop_back();
                _blockSizes.pop_back();
            }
        }
    }

    virtual void readCharArray( char* s, unsigned int size )
    { if ( size>0 ) _in->read( s, size ); }

    virtual void readWrappedString( std::string& str )
    { readString( str ); }

    virtual void advanceToCurrentEndBracket()
    {
        if ( _supportBinaryBrackets && _beginPositions.size()>0 )
        {
            std::streampos position(_beginPositions.back());
            position += _blockSizes.back();
            _in->seekg( position );
            _beginPositions.pop_back();
            _blockSizes.pop_back();
        }
    }

protected:
    std::vector<std::streampos> _beginPositions;
    std::vector<int> _blockSizes;
};

#endif
