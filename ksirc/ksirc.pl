#
# $$Id: ksirc.pl 76885 2001-01-08 07:36:43Z malte $$
#

&addhelp("Ban",
"\cbAdded by KSirc.pl\cb
Usage: BAN <nickname>
bans the specified user on the current channel. Only channel operators
can use this command. Bans the user in the form *!*user\@hostmask.  
hostmask is xxx.xxx.xxx.* if the hostname is in dotted quad form, otherwise 
it is *.domain.com
See Also: UNBAN");

&addhelp("UnBan",
"\cbAdded by KSirc.pl\cb
Usage: UNBAN <nickname>
Unbans the specified user on the given channel. Only channel operators
can use this command.  See BAN");

&addhelp("ClrBan",
"\cbAdded by KSirc.pl\cb
Usage: CLRBAN [<#channel>]
Removes ALL bans from the given channel. Only channel operators can use
this command.  The channel defaults to your current one.
See Also: MODE [-b]");

&addhelp("FC",
"\cbAdded by KSirc.pl\cb
Usage: FC [<#channel>] <Filter> <command>
Does /names on the given channel.  Uses the current channel if none 
specified. Does a userhost on each person received. if their name 
(in form nick!user\@host) matches your filter (in perl regex form) 
then command is executed, in command $1 is expanded to the nick of 
the matchee.
Examples: 
  /fc #dragonrealm *!*\@*.com msg $1 you are on a host that ends in .com
  /fc *!*\@((\\d+)\\.){3}\\.(\\d+) say $1 has a numeric host.");

&addhelp("Pig",
"\cbAdded by KSirc.pl\cb
Usage: PIG <message>
Translates your message into piglatin and says it on the current channel.");

&addhelp("WallOP",
"\cbAdded by KSirc.pl\cb
Usage: WALLOP [<#channel>] <message>
Sends a message to all of the channel operators on the given channel.  
Defaults the the current channel.");

sub cmd_wallop {
  &getarg;
  unless ($newarg =~ /^#/) {
    $args = $newarg." ".$args;
    $newarg = $talkchannel;
  }
  &notice("\@$newarg","[KSirc-Wall/$newarg]: $args"); 
}
&addcmd("wallop");


sub modeb {
  ($which) = @_;
  $user =~ s/^~//;
  if (length($user) > 8) {
    $user = substr($user,0, 7);
    $user .= "*";
  }
  @quad = split(/\./, $host);
  if ($host =~ /(\d+)\.(\d+)\.(\d+)\.(\d+)/) {
    pop @quad;
    $toban = join('.', @quad);
    $toban .= ".*";
  } else {
    $toban = "*.".$quad[@quad-2].".".$quad[@quad-1];
  }
  &docommand("mode $talkchannel ${which}b *!*$user*\@$toban");
}

sub cmd_ban {
  &getarg;
  if ($newarg) {
    &userhost($newarg, "&modeb(\"+\");");
  } else {
    &tell("~4Usage: /ban <nick>.");
  }
}
&addcmd("ban");

sub cmd_unban {
  &getarg;
  if ($newarg) {
    &userhost($newarg, "&modeb(\"-\");");
  } else {
    &tell("~4Usage: /unban <nick>.");
  }
}
&addcmd("unban");

sub cmd_k {
  &getarg;
  $args = "You have been kicked by a KSirc user." unless $args;
  if ($newarg) {
    &docommand("kick $talkchannel $newarg $args");
  } else {
    &tell("~4Usage: /k <nick> [reason]");
  }
}
&addcmd("k");

sub cmd_kb {
  &getarg;
  if ($newarg) {
    &docommand("ban $newarg");
    &docommand("k $newarg $args");
  } else {
    &tell("~4Usage: /kb <nick> [reason]");
  }
}
&addcmd("kb");

sub cmd_clrban {
  &getarg;
  $newarg = $talkchannel unless $newarg;
  &addhook("367", "tban"); 
  &addhook("368","rm367");
  &docommand("mode $newarg b");
}
&addcmd("clrban");

sub hook_tban {
  $silent = 1;
  my ($shit, $channel, $mask, $banner, $time) = split(/ +/, $_[0]);
  push @bans, $mask;
  if (@bans == 6) {
    &print("mode $channel -bbbbbb @bans");
    @bans = ();
  }
}

sub hook_rm367 {
  @bans = ();
  &remhook("367","tban");
  &remhook("368","rm367");
}

sub hook_disconnectd {
  &docommand("server 1");
}
&addhook("disconnect","disconnectd");

#sub hook_kickd {
#  &docommand("join $_[1]") if $_[0] eq $nick;
#}
#&addhook("kick","kickd");

sub cmd_fcmd {
  ($names,$mask,$command) = split(/ /, $args,3);
  $mask =~ s/\!/\!/;
  $mask =~ s/\@/\@/;
  $mask =~ s/\./\\./g;
  $mask =~ s/\*/.*/g;
  &addhook("353","filtercommand");
  &addhook("366","removefiltercommand");
  &tell("\t\cb~4Matching /$mask/i on $names...");
  &docommand("names $names");
}
&addcmd("fcmd");

sub hook_filtercommand {
  ($shit, $_[0]) = split(/:/, $_[0]);
  my @names = split(/ /, $_[0]);
  for (@names) {
    $_ =~ s/^\@//;
    &userhost($_, "&dofilter");
  }
  $silent=1;
}

sub dofilter {
  $s = "$who\!$user\@$host";
  #&tell("$s =~ /$mask/");
  if ($s =~ /$mask/i) {
    $d = $command;
    $d =~ s/\$1/$who/;
    &docommand($d);
  }
}

sub hook_removefiltercommand {
  &remhook("353","filtercommand");
  &remhook("366","removefiltercommand");
  &tell("~4Filter on $names, /$mask/i, Done.");
}

sub cmd_fc {
  my ($mask, $cmd) = split(/ /, $args, 2);
  &docommand("fcmd $talkchannel $mask $cmd");
}
&addcmd("fc");

sub cmd_pig {
  $_[0] =~ s/^pig //i;
  &say(&topiglatin($_[0]));
}
&addcmd("pig");

sub topiglatin {
@words = split(/ /, $_[0]);
for (@words) {
  if ($_ =~ /^([bcdfghjklmnpqrstvwxyzBCDFGHJKLMNPQRSTVWXYZ])([aeiouAEIOU])/) {
    $_ .= substr($_,0,1)."ay";
    $_ = substr($_,1);
  } elsif ($_ =~ /^([bcdfghjklmnpqrstvwxyzBCDFGHJKLMNPQRSTVWXYZ])([bcdfghjklmnpqrstvwxyzBCDFGHJKLMNPQRSTVWXYZ])/) {
    $_ .= $1.$2."ay";
    $_ = substr($_,2);
  } elsif ($_ =~ /^[aeiouAEIOU]/) {
    $_ .= "way";
  } else {
    print "shouldn't print me\n";
  }
}
return "@words";
}

&addhelp("follow",
"\cbAdded by KSirc.pl\cb
Usage: follow <nick>
Highlight <nick> in colour when ever they say anything");

&addhelp("unfollow",
"\cbAdded by KSirc.pl\cb
Usage: unfollow <nick>
Stop highlighting the nick");

&addhelp("showfollows",
"\cbAdded by KSirc.pl\cb
Usage: showfollows
Shows who you are currently following");

#### Start follow script from caracas

&addcmd ('follow');
&addcmd ('unfollow');
&addcmd ('showfollows');


@follow_colors = ( '~1', '~2', '~3', '~4', '~5', '~6', '~7', '~8', '~9', '~10', '~11', '~12', '~13', '~14', '~15' );
undef %following;


sub cmd_follow
{
   my ($fnick) = shift;
   my ($color);

   $fnick =~ s/^follow\s+//;
   if (defined ($following {lc ($fnick)}))
   {
      &tell ("Already following " . $following {lc ($fnick)});
   }
   else
   {
      $color = $follow_colors [int (rand scalar (@follow_colors))];
      &docommand ("^ksircappendrule DESC==Follow $fnick !!! " .
                  "SEARCH==<\\S*$fnick\\S*> !!! FROM==<\\S*($fnick)\\S*> !!! TO==\"<$color\$1~c>\"");
      $following {lc ($fnick)} = "${color}${fnick}~c";
      &tell ("Following ${color}$fnick~c ...");
   }
}


sub cmd_unfollow
{
   my ($fnick) = shift;
   my ($filter);

   $fnick =~ s/^unfollow\s+//;
   for ($filter = 0;   $filter <= $#KSIRC_FILTER;   $filter++)
   {
      if ($KSIRC_FILTER [$filter]{'DESC'} =~ /Follow $fnick/i)
      {
         &docommand ("^ksircdelrule $filter");
         delete ($following{ lc $fnick});
         &tell ("$fnick no longer followed");
         return;
      }
   }
   &tell ("Wasn't following $fnick");
}


sub cmd_showfollows
{
   my ($fnick);

   if (scalar (keys %following) > 0)
   {
      foreach $fnick (sort keys %following)
      {
         &tell ("Following " . $following {$fnick});
      }
   }
   else
   {
      &tell ("Not currently following anyone");
   }
}

#### End follow

sub cmd_refresh
{
  &tell("*** Refresh nick list");
  &docommand("names *");
}
&addcmd("refresh");

sub hook_url_who_list {
    my @info = split(/\s+/, $_[0]);
    &print("*I* URL: http://$info[3]:<port>/");
    $silent = 1;
}

sub hook_url_end_who {
    &remhook("352", "url_who_list");
    &remhook("315", "url_end_who");
    $args = "";
}

&addhelp("url",
"\cbAdded by KSirc.pl\cb
Usage: URL
Prints out your url");


sub cmd_url
{
    &addhook("352", "url_who_list");
    &addhook("315", "url_end_who");
    &sl("who :$nick");
}
&addcmd("url");

sub cmd_extnames
{
  if($who_active == 0){
    &addhook("352", "ksirc_who_list");
    &addhook("315", "ksirc_who_end");
  }
  &dosplat;
  &getarg;
  &sl("who :$newarg");
  $who_active++;  
  $WHO_INFO{uc($newarg)} = "";
  $WHO_TIME{uc($newarg)} = 0;
}
&addcmd("extnames");

sub hook_ksirc_who_end {
  $who_active--;
  if($who_active == 0){
    &remhook("352", "ksirc_who_list");
    &remhook("315", "ksirc_who_end");
  }
  my @info = split(/\s+/, $_[0]);
  # 0: our nick
  # 1: channel
  # 2 Onwards: misc info
  $info[1] = uc($info[1]);
  chop($WHO_INFO{$info[1]}); # Remove trailing space
  my $c = ($WHO_TIME{$info[1]} == 0) ? "C" : "!";
  if(length($WHO_INFO{$info[1]}) > 0){
    &print("~$info[1]~*$c* Users on $info[1]: $WHO_INFO{$info[1]}");
  }
  &print("~$info[1]~*c* Done Parsing Who");

  delete($WHO_INFO{$info[1]});
  delete($WHO_TIME{$info[1]});
  $args = "";
}

sub hook_ksirc_who_list {
  my @info = split(/\s+/, $_[0]);
  # 0: our nick
  # 1: channel
  # 2: ident
  # 3: one server
  # 4: another server
  # 5: nick
  # 6: status
  # 7: rest is useless
  $silent = 1;
  my $who_nick = $info[5];
  #    print "*I* Parsing: $_[0], info6: $info[6]\n";
  if($info[6] =~ /G/){
    $who_nick = "#" . $who_nick;
  }
  if($info[6] =~ /\+/){
    $who_nick = "+" . $who_nick;
  }
  if($info[6] =~ /\@/){
    $who_nick = "@" . $who_nick;
  }
  if($info[6] =~ /\*/){
    $who_nick = "*" . $who_nick;
  }

  $info[1] = uc($info[1]);
  
  $WHO_INFO{$info[1]} .= $who_nick . " ";
  if(length($WHO_INFO{$info[1]}) > 512){
    my $c = ($WHO_TIME{$info[1]} == 0) ? "C" : "!";
    &print("~$info[1]~*$c* Users on $info[1]: $WHO_INFO{$info[1]}");
    $WHO_INFO{$info[1]} = "";
    $WHO_TIME{$info[1]}++;
  }
}

&tell("*** \0032,4\cbLoaded KSirc.pl\003");
&tell("*** \00313,3\cbWith: Super Willy Enhancements, LotR's exec\003");
sub cmd_exec {

	my $how, $to;

	&getarg;
	$how = "x";
	if (&eq($newarg, "-OUT")) { $how = 'c'; }
	if (&eq($newarg, "-MSG")) { $how = 'm'; &getarg; $to = $newarg; }
	if (&eq($newarg, "-NOTICE")) { $how = 'n'; &getarg; $to = $newarg; }
	if ($how eq "x") { $args = $newarg . " " . $args; }
	open (CMD, "$args|");
	while (<CMD>) {
		chomp;
		if ($how eq 'c') {
			&say(" $_");
		} elsif ($how eq 'm') {
			&msg($to, $_);
		} elsif ($how eq 'n') {
			&notice($to, $_);
		} else {
			print "$_\n";
		}
	}
	close CMD;
}

&addcmd("exec");
&addhelp("exec", "Usage: EXEC <shell commands>\n" .
       "EXEC -OUT <shell commands]\n" .
       "EXEC -MSG <nickname> <shell commands>]\n" .
       "EXEC -NOTICE <nickname> <shell commands>]\n" );

sub hook_fixcolours {
# those keycodes correspond to the ones in ahistlineedit.cpp, keyPressEvent()
  $_[1] =~ tr/[\336\327\237\252\244]/\[\002\037\026\003\170]/;
}

&addhook("send_text", "fixcolours");

sub cmd_help {
  &tell("*\cbH\cb* Help not available"), return unless @help;
  my $found ='';

  &getarg;
  if($newarg =~ /^\s*$/){
    my $line = '';
    my %once;
    foreach (@help) {  
      if (/^\@/) {
        if (&eq($_, "\@main")) {
          $found=1;
          &tell("*\cbH\cb* Help on $newarg") if $newarg ne 'main';  # KSIRC MOD
        }          
        else {     
          $found = 0;
          my $cmd = /\@(\S+)/;
          next if $once{$1};
          $once{$1} = 1;
          $line .= "$1 " . " "x(15-length("$1 "));  # KSIRC MOD
          if(length($line) > 50){
            &tell("*\cbH\cb* $line");
            $line = "";              
          }                          
        }                            
      } else {                       
        &tell("*\cbH\cb* $_") if $found;
      }                               
    }
    &tell("*\cbH\cb* $line");         
  }                                   
  else{                               
    $newarg =~ s/ *$//;
    foreach (@help) {
      if (/^\@/) {
        last if $found;
        if (&eq($_, "\@$newarg")) {
          $found=1;
          &tell("*\cbH\cb* Help on $newarg") if $newarg ne 'main';
        }
      } else {
        &tell("*\cbH\cb* $_") if $found;
      }
    }
  }                                    # KSIRC MOD
  &tell("*\cbH\cb* Unknown help topic; try /help") unless $found;
}

&addcmd("help");
