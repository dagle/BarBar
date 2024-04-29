# Conky

This is an example in how to create a [conky](https://github.com/brndnmtthws/conky) config
with barbar. While barbar and conky isn't constructed the same way
similar results can be achieved. We will base our example on the
[Conky 4K](https://gist.github.com/nbicocchi/6a356e87a1543abb15e6855edf7c9826#file-conky-conf)
from the conky wiki. It was chosen because it's fairly simple
and close to the default config while containing some extra feature
compared to the default.

This only an example, it doesn't document every feature in barbar / glib / gtk,
it's merely a starting point.

# Getting started

First thing you notice is that cony uses a single file config (which can be split up),
where barbar uses 2-3 files. The main difference is that barbar splits between components
and styling. In gtk, styling is done with css.


