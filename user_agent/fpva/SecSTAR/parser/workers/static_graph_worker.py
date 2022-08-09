"""
Generate static graph
"""

import logging
import os

from structures import lookup_table
from structures import event
from structures import graph_artist

def run(table, output_dir, dedup=False):

    logging.info('[3] Static graph is being generated ...')

    artist = graph_artist.GraphArtist(table, dedup, static=True)
    artist.draw(output_dir)
