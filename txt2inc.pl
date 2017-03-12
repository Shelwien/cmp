
sub getdate {
  my ($s,$m,$h,$d,$mn,$y) = localtime(time);
  return sprintf "%02i.%02i.%04i %02i:%02i", $d,$mn+1,$y+1900, $h,$m;
}

$curdate = getdate();

open( I, "<$ARGV[0]" ) || die;
open( O, ">$ARGV[1]" ) || die;

$flag=0;
print O "\n";
while( <I> ) {
  if( /\[\[(.*)\]\]/ ) {
    print O ";\n\n" if $flag;
    print O "char $1\[\] = \n"; $flag=1;
  } else {
    s/[\r\n]//g;
    s/\s+$//;
    if( /^;$/ ) {
    } else {
      s/\\/\\\\/g;
      s/"/\\"/g;
      s/%/%%/g;
      s/\$DATA/%s/g;
      s/\$DATX/%X/g;
      s/\$DATE/$curdate/g;
      print O "\"$_\\n\"\n";
    }
  }
}
print O ";\n" if $flag;
