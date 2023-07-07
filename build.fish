#!/usr/bin/fish

gcc -shared -fPIC uri-parser.c -luriparser -lcgab -o libcgaburi-parser.so

if [ -e libcgaburi-parser.so ]
    echo "libcgaburi-parser.so created"
else
    echo "libcgaburi-parser.so not created"
end

if [ "$argv[1]" = "install" ]
    sudo cp libcgaburi-parser.so /usr/lib/
    echo "libcgabhttp-parser installed"
end
