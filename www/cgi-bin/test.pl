#!/usr/bin/perl

use strict;
use warnings;

print "Content-Type: text/plain\r\n\r\n";

print "Hello from Perl CGI!\n\n";
print "Environment variables:\n";
print "----------------------\n";

foreach my $key (sort keys %ENV) {
    print "$key = $ENV{$key}\n";
}
