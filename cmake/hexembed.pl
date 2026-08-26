#!/usr/bin/env perl
use strict;
use warnings;

die "Usage: $0 <input> <output> <symbol>\n" unless @ARGV == 3;

my ($input, $output, $symbol) = @ARGV;

open my $in, '<:raw', $input or die "Cannot read $input: $!\n";
local $/;
my $data = <$in>;
close $in;

my @bytes = unpack('C*', $data);
my $count = scalar @bytes;

open my $out, '>', $output or die "Cannot write $output: $!\n";
print {$out} "unsigned char ${symbol}[] = {\n";

for my $i (0 .. $#bytes) {
    print {$out} '  ' if ($i % 12) == 0;
    print {$out} sprintf('0x%02x', $bytes[$i]);
    if ($i == $#bytes) {
        print {$out} "\n";
    } elsif (($i + 1) % 12 == 0) {
        print {$out} ",\n";
    } else {
        print {$out} ', ';
    }
}

print {$out} "};\n";
print {$out} "unsigned int ${symbol}_len = ${count};\n";
close $out;
