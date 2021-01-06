
&::PukeSendMessage($::PUKE_WIDGET_LOAD,
                     $::PUKE_CONTROLLER,
                     $::PWIDGET_KSIRCLISTBOX,
                     "ksirclistbox.so",
                     sub { my %ARG = %{shift()};
                     if($ARG{'iArg'} == 1){
                       print "*E* PKSircListBox Load failed!\n";
                     }
                     }
                  );


package PKSircListBox; 
@ISA = qw(PListBox);
use strict;

sub new { 
  my $class = shift;
  my $self = $class->SUPER::new($class, @_);

  $self->{widgetType} = $::PWIDGET_KSIRCLISTBOX;

  if($class eq 'PKSircListBox'){
    $self->create();
  }

  return $self;

}

sub DESTROY { 
    my $self = shift;
    $self->SUPER::DESTROY(@_);
}


sub scrollToBottom {
  my $self = shift;

  my $force = shift;
  
  $self->sendMessage('iCommand' => $::PUKE_KSIRCLISTBOX_TOBOTTOM,
                     'iArg' => $force,
                     'CallBack' => sub {});

}



package main; 

1;
