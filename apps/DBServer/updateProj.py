#!/usr/bin/env python3

import sys

sys.path.append('../')
import projBuilder as pb

pb.Build( sys.argv, 'proto' )
