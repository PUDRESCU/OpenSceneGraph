#ifndef IM_SIZE_REDUCTION

#ifndef OSGDB_ASCIISTREAMOPERATOR
#define OSGDB_ASCIISTREAMOPERATOR

#include <ostream>
#include <osgDB/StreamOperator>

class AsciiOutputIterator : public osgDB::OutputIterator
{
public:
    AsciiOutputIterator( std::ostream* ostream, int precision )
    : _readyForIndent(false), _indent(0)
    {
        _out = ostream;
        if (precision>0) _out->precision(precision);
    }

    virtual ~AsciiOutputIterator() {}

    virtual bool isBinary() const { return false; }

    virtual void writeBool( bool b )
    {
#ifdef IM_SIZE_REDUCTION
#else
        indentIfRequired();
        if ( b ) *_out << "TRUE ";
        else *_out << "FALSE ";
#endif
    }

    virtual void writeChar( char c )
    {
#ifdef IM_SIZE_REDUCTION
#else
      indentIfRequired(); *_out << (short)c << ' ';
#endif
    }

    virtual void writeUChar( unsigned char c )
    {
#ifdef IM_SIZE_REDUCTION
#else
      indentIfRequired(); *_out << (unsigned short)c << ' ';
#endif
    }

    virtual void writeShort( short s )
    {
#ifdef IM_SIZE_REDUCTION
#else
      indentIfRequired(); *_out << s << ' ';
#endif
    }

    virtual void writeUShort( unsigned short s )
    {
#ifdef IM_SIZE_REDUCTION
#else
      indentIfRequired(); *_out << s << ' ';
#endif
    }

    virtual void writeInt( int i )
    {
#ifdef IM_SIZE_REDUCTION
#else
      indentIfRequired(); *_out << i << ' ';
#endif
    }

    virtual void writeUInt( unsigned int i )
    {
#ifdef IM_SIZE_REDUCTION
#else
      indentIfRequired(); *_out << i << ' ';
#endif
    }

    virtual void writeLong( long l )
    {
#ifdef IM_SIZE_REDUCTION
#else
      indentIfRequired(); *_out << l << ' ';
#endif
    }

    virtual void writeULong( unsigned long l )
    {
#ifdef IM_SIZE_REDUCTION
#else
      indentIfRequired(); *_out << l << ' ';
#endif
    }

    virtual void writeFloat( float f )
    {
#ifdef IM_SIZE_REDUCTION
#else
      indentIfRequired(); *_out << f << ' ';
#endif
    }

    virtual void writeDouble( double d )
    {
#ifdef IM_SIZE_REDUCTION
#else
      indentIfRequired(); *_out << d << ' ';
#endif
    }

    virtual void writeString( const std::string& s )
    {
#ifdef IM_SIZE_REDUCTION
#else
      indentIfRequired(); *_out << s << ' ';
#endif
    }

    virtual void writeStream( std::ostream& (*fn)(std::ostream&) )
    {
#ifdef IM_SIZE_REDUCTION
#else
        indentIfRequired(); *_out << fn;
        if ( isEndl( fn ) )
        {
            _readyForIndent = true;
        }
#endif
    }

    virtual void writeBase( std::ios_base& (*fn)(std::ios_base&) )
    {
#ifdef IM_SIZE_REDUCTION
#else
        indentIfRequired(); *_out << fn;
#endif
    }

    virtual void writeGLenum( const osgDB::ObjectGLenum& value )
    {
#ifdef IM_SIZE_REDUCTION
#else
        GLenum e = value.get();
        const std::string& enumString = osgDB::Registry::instance()->getObjectWrapperManager()->getString("GL", e);
        indentIfRequired(); *_out << enumString << ' ';
#endif
    }

    virtual void writeProperty( const osgDB::ObjectProperty& prop )
    {
#ifdef IM_SIZE_REDUCTION
#else
        std::string enumString = prop._name;
        if ( prop._mapProperty )
        {
            enumString = osgDB::Registry::instance()->getObjectWrapperManager()->getString(prop._name, prop._value);
        }
        indentIfRequired(); *_out << enumString << ' ';
#endif
    }

    virtual void writeMark( const osgDB::ObjectMark& mark )
    {
#ifdef IM_SIZE_REDUCTION
#else
        _indent += mark._indentDelta;
        indentIfRequired(); *_out << mark._name;
#endif
    }

    virtual void writeCharArray( const char* /*s*/, unsigned int /*size*/ ) {}

    virtual void writeWrappedString( const std::string& str )
    {
#ifdef IM_SIZE_REDUCTION
#else
        std::string wrappedStr;
        unsigned int size = str.size();
        for ( unsigned int i=0; i<size; ++i )
        {
            char ch = str[i];
            if ( ch=='\"' ) wrappedStr += '\\';
            else if ( ch=='\\' ) wrappedStr += '\\';
            wrappedStr += ch;
        }

        wrappedStr.insert( std::string::size_type(0), 1, '\"' );
        wrappedStr += '\"';

        indentIfRequired();
        writeString( wrappedStr );
#endif
    }

protected:

    inline void indentIfRequired()
    {
        if ( _readyForIndent )
        {
            for (int i=0; i<_indent; ++i)
                *_out << ' ';
            _readyForIndent = false;
        }
    }

    bool _readyForIndent;
    int _indent;
};

class AsciiInputIterator : public osgDB::InputIterator
{
public:
    AsciiInputIterator( std::istream* istream ) { _in = istream; }
    virtual ~AsciiInputIterator() {}

    virtual bool isBinary() const { return false; }

    virtual void readBool( bool& b )
    {
        std::string boolString;
        readString( boolString );
        if ( boolString=="TRUE" ) b = true;
        else b = false;
    }

    virtual void readChar( char& c )
    {
        short s = 0;
        readShort( s );
        c = (char)s;
    }

    virtual void readSChar( signed char& c )
    {
        short s = 0;
        readShort( s );
        c = (signed char)s;
    }

    virtual void readUChar( unsigned char& c )
    {
        short s = 0;
        readShort( s );
        c = (unsigned char)s;
    }

    virtual void readShort( short& s )
    { std::string str; readString(str); s = static_cast<short>(strtol(str.c_str(), NULL, 0)); }

    virtual void readUShort( unsigned short& s )
    { std::string str; readString(str); s = static_cast<unsigned short>(strtoul(str.c_str(), NULL, 0)); }

    virtual void readInt( int& i )
    { std::string str; readString(str); i = static_cast<int>(strtol(str.c_str(), NULL, 0)); }

    virtual void readUInt( unsigned int& i )
    { std::string str; readString(str); i = static_cast<unsigned int>(strtoul(str.c_str(), NULL, 0)); }

    virtual void readLong( long& l )
    { std::string str; readString(str); l = strtol(str.c_str(), NULL, 0); }

    virtual void readULong( unsigned long& l )
    { std::string str; readString(str); l = strtoul(str.c_str(), NULL, 0); }

    virtual void readFloat( float& f )
    { std::string str; readString(str); f = osg::asciiToFloat(str.c_str()); }

    virtual void readDouble( double& d )
    { std::string str; readString(str); d = osg::asciiToDouble(str.c_str()); }

    virtual void readString( std::string& s )
    {
        if ( _preReadString.empty() )
            *_in >> s;
        else
        {
            s = _preReadString;
            _preReadString.clear();
        }
    }

    virtual void readStream( std::istream& (*fn)(std::istream&) )
    { *_in >> fn; }

    virtual void readBase( std::ios_base& (*fn)(std::ios_base&) )
    { *_in >> fn; }

    virtual void readGLenum( osgDB::ObjectGLenum& value )
    {
        GLenum e = 0;
        std::string enumString;
        readString( enumString );
        e = osgDB::Registry::instance()->getObjectWrapperManager()->getValue("GL", enumString);
        value.set( e );
    }

    virtual void readProperty( osgDB::ObjectProperty& prop )
    {
        int value = 0;
        std::string enumString;
        readString( enumString );
        if ( prop._mapProperty )
        {
            value = osgDB::Registry::instance()->getObjectWrapperManager()->getValue(prop._name, enumString);
        }
        else
        {
            if ( prop._name!=enumString )
            {
                OSG_WARN << "AsciiInputIterator::readProperty(): Unmatched property "
                                       << enumString << ", expecting " << prop._name << std::endl;
            }
            prop._name = enumString;
        }
        prop.set( value );
    }

    virtual void readMark( osgDB::ObjectMark& /*mark*/ )
    {
        std::string markString;
        readString( markString );
    }

    virtual void readCharArray( char* /*s*/, unsigned int /*size*/ ) {}

    virtual void readWrappedString( std::string& str )
    {
        char ch;
        getCharacter( ch );

        // skip white space
        while ( ch==' ' || (ch=='\n') || (ch=='\r'))
        {
            getCharacter( ch );
        }

        if (ch=='"')
        {
            // we have an "wrapped string"
            getCharacter( ch );
            while ( ch!='"' )
            {
                if (ch=='\\')
                {
                    getCharacter( ch );
                    str += ch;
                }
                else str += ch;

                getCharacter( ch );
            }
        }
        else
        {
            // we have an unwrapped string, read to first space or end of line
            while ( (ch!=' ') && (ch!=0) && (ch!='\n') )
            {
                str += ch;
                getCharacter( ch );
            }
        }
    }

    virtual bool matchString( const std::string& str )
    {
        if ( _preReadString.empty() )
            *_in >> _preReadString;

        if ( _preReadString==str )
        {
            _preReadString.clear();
            return true;
        }
        return false;
    }

    virtual void advanceToCurrentEndBracket()
    {
        std::string passString;
        unsigned int blocks = 0;
        while ( !_in->eof() )
        {
            passString.clear();
            readString( passString );

            if ( passString=="}" )
            {
                if ( blocks<=0 ) return;
                else blocks--;
            }
            else if ( passString=="{" )
                blocks++;
        }
    }

protected:
    void getCharacter( char& ch )
    {
        if ( !_preReadString.empty() )
        {
            ch = _preReadString[0];
            _preReadString.erase( _preReadString.begin() );
        }
        else
        {
            _in->get( ch );
            checkStream();
        }
    }

    std::string _preReadString;
};

#endif
#endif
