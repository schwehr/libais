"""Convert libais message dictionaries to GPSD JSON."""

import datetime


class Mangler(object):
  """Convert libais dictionaries to gpsd dictionaries."""

  def __init__(self, copy_tagblock_timestamp=True):
    self.copy_tagblock_timestamp = copy_tagblock_timestamp

  def __call__(self, msg):
    res = {}
    self.mangle(res, msg)
    method = 'mangle__%s' % (msg['id'],)
    if hasattr(self, method):
      getattr(self, method)(res, msg)
    for key in msg:
      method1 = 'mangle__%s__%s' % (key, msg['id'])
      method2 = 'mangle__%s' % (key,)
      if hasattr(self, method1):
        getattr(self, method1)(res, msg)
      elif hasattr(self, method2):
        getattr(self, method2)(res, msg)
      else:
        res[key] = msg[key]
    return res

  def mangle(self, res, msg):
    res['class'] = 'AIS'
    res['device'] = 'stdin'
    res['scaled'] = True
    if msg['id'] in (1, 2, 3):
      res['status'] = '15'
      res['status_text'] = self.nav_statuses[15]
      res['heading'] = 511

  def mangle__id(self, res, msg):
    res['type'] = msg['id']

  # Types 1, 2 and 3: Position Report Class A.

  def mangle__cog(self, res, msg):
    res['course'] = msg['cog']

  def mangle__nav_status(self, res, msg):
    res['status'] = msg['nav_status']
    res['status_text'] = self.nav_statuses[msg['nav_status']]

  def mangle__position_accuracy(self, res, msg):
    res['accuracy'] = msg['position_accuracy'] == 1

  def mangle__repeat_indicator(self, res, msg):
    res['repeat'] = msg['repeat_indicator']

  def mangle__rot_over_range(self, res, msg):
    pass

  def mangle__rot(self, res, msg):
    if msg['rot_over_range']:
      res['turn'] = None
    else:
      res['turn'] = msg['rot']

  def mangle__sog(self, res, msg):
    res['speed'] = msg['sog']

  def mangle__special_manoeuvre(self, res, msg):
    res['maneuver'] = msg['special_manoeuvre']

  # spare, special_manoeuvre, sync_state, timestamp.

  def mangle__true_heading(self, res, msg):
    res['heading'] = msg['true_heading']

  # utc_hour, utc_min, utc_spare.

  def mangle__x(self, res, msg):
    res['lon'] = msg['x']

  def mangle__y(self, res, msg):
    res['lat'] = msg['y']

  # Type 4: Base Station Report

  def mangle__year(self, res, msg):
    res['timestamp'] = '%04d-%02d-%02dT%02d:%02d:%02dZ' % (msg['year'],
                                                           msg['month'],
                                                           msg['day'],
                                                           msg['hour'],
                                                           msg['minute'],
                                                           msg['second'])

  def mangle__month(self, res, msg):
    pass

  def mangle__day(self, res, msg):
    pass

  def mangle__hour(self, res, msg):
    pass

  def mangle__minute(self, res, msg):
    pass

  def mangle__second(self, res, msg):
    pass

  def mangle__fix_type(self, res, msg):
    res['epfd'] = msg['fix_type']
    res['epfd_text'] = self.fix_types.get(msg['fix_type'], self.fix_types[0])

  # Type 5: Static and Voyage Related Data  #####

  def mangle__name(self, res, msg):
    res['shipname'] = msg['name'].strip('@').strip()

  def mangle__destination(self, res, msg):
    res['destination'] = msg['destination'].strip('@').strip()

  def mangle__callsign(self, res, msg):
    res['callsign'] = msg['callsign'].strip('@').strip()

  def mangle__dim_a(self, res, msg):
    res['to_bow'] = msg['dim_a']

  def mangle__dim_b(self, res, msg):
    res['to_stern'] = msg['dim_b']

  def mangle__dim_c(self, res, msg):
    res['to_port'] = msg['dim_c']

  def mangle__dim_d(self, res, msg):
    res['to_starboard'] = msg['dim_d']

  def mangle__eta_day(self, res, msg):
    if msg['eta_month'] < 1 or msg['eta_day'] == 0 or msg['eta_hour'] == 24:
      return
    if msg['eta_minute'] == 60:
      return

    year = 0
    if 'year' in msg:
      year = msg['year']
    elif 'tagblock_timestamp' in msg:
      year = datetime.datetime.utcfromtimestamp(msg['tagblock_timestamp']).year

    try:
      eta = datetime.datetime(year,
                              msg['eta_month'],
                              msg['eta_day'],
                              msg['eta_hour'],
                              msg['eta_minute'])
    except:
      # TODO(redhog): What exception is being triggered and why?
      pass
    else:
      res['eta'] = eta.strftime('%Y-%m-%dT%H:%H:%S.%fZ')

  def mangle__eta_hour(self, res, msg):
    pass

  def mangle__eta_minute(self, res, msg):
    pass

  def mangle__eta_month(self, res, msg):
    pass

  def mangle__imo_num(self, res, msg):
    res['imo'] = str(msg['imo_num'])

  def mangle__type_and_cargo(self, res, msg):
    res['shiptype'] = msg['type_and_cargo']
    try:
      res['shiptype_text'] = self.ship_types[msg['type_and_cargo']]
    except KeyError:
      res['shiptype_text'] = '%d - Unknown' % msg['type_and_cargo']

  # Type 6: Binary Addressed Message

  def mangle__mmsi_dest(self, res, msg):
    res['dest_mmsi'] = msg['mmsi_dest']

  def mangle__seq(self, res, msg):
    res['seqno'] = msg['seq']

  # Note: retransmit has different values for the same message from gpsd... bug?

  # Type 7: Binary Acknowledge

  def mangle__7(self, res, unused_msg):
    """Values will be set in mangle__acks."""
    res.update({
        'mmsi1': 0,
        'mmsi2': 0,
        'mmsi3': 0,
        'mmsi4': 0})

  def mangle__acks(self, res, msg):
    for idx, (mmsi, unused_seq_num) in enumerate(msg['acks']):
      res['mmsi%s' % (idx+1,)] = mmsi

  # Type 8: Binary Broadcast Message

  def mangle__fi(self, res, msg):
    res['fid'] = msg['fi']

  # Note: Data is missing from libais message

  # Type 9: Standard SAR Aircraft Position Report

  def mangle__timestamp(self, res, msg):
    res['second'] = msg['timestamp']

  # Type 12: Addressed Safety-Related Message

  def mangle__seq_num(self, res, msg):
    res['seqno'] = msg['seq_num']

  def mangle__retransmitted(self, res, msg):
    res['retransmit'] = msg['retransmitted']

  # Type 13: Safety-Related Acknowledgement

  def mangle__13(self, res, unused_msg):
    res.update({
        'mmsi1': 0,
        'mmsi2': 0,
        'mmsi3': 0,
        'mmsi4': 0})

  # Type 15: Interrogation

  def mangle__15(self, res, unused_msg):
    res.update({
        'mmsi1': 0,
        'offset1_1': 0,
        'type1_1': 0,
        'offset1_2': 0,
        'type1_2': 0,
        'mmsi2': 0,
        'offset2_1': 0,
        'type2_1': 0,
        'offset2_2': 0,
        'type2_2': 0})

  def mangle__mmsi_1(self, res, msg):
    res['mmsi1'] = msg['mmsi_1']

  def mangle__mmsi_2(self, res, msg):
    res['mmsi2'] = msg['mmsi_2']

  def mangle__slot_offset_1_1(self, res, msg):
    res['offset1_1'] = msg['slot_offset_1_1']

  def mangle__slot_offset_1_2(self, res, msg):
    res['offset1_2'] = msg['slot_offset_1_2']

  def mangle__msg_1_1(self, res, msg):
    res['type1_1'] = msg['msg_1_1']

  def mangle__dest_msg_1_2(self, res, msg):
    res['type1_2'] = msg['dest_msg_1_2']

  def mangle__msg_2_1(self, res, msg):
    res['type2_1'] = msg['msg_2_1']

  def mangle__dest_msg_2_2(self, res, msg):
    res['type2_2'] = msg['dest_msg_2_2']

  # Type 16: Assignment Mode Command

  def mangle__16(self, res, unused_msg):
    res.update({
        'increment1': 0,
        'offset1': 0,
        'mmsi1': 0,
        'increment2': 0,
        'offset2': 0,
        'mmsi2': 0})

  def mangle__inc_a(self, res, msg):
    res['increment1'] = msg['inc_a']

  def mangle__dest_mmsi_a(self, res, msg):
    res['mmsi1'] = msg['dest_mmsi_a']

  def mangle__offset_a(self, res, msg):
    res['offset1'] = msg['offset_a']

  def mangle__inc_b(self, res, msg):
    res['increment2'] = msg['inc_b']

  def mangle__dest_mmsi_b(self, res, msg):
    res['mmsi2'] = msg['dest_mmsi_b']

  def mangle__offset_b(self, res, msg):
    res['offset2'] = msg['offset_b']

  # Type 17: DGNSS Broadcast Binary Message

  # Note: Data is missing from libais message

  # Type 18: Standard Class B CS Position Report #####

  def mangle__band_flag(self, res, msg):
    res['band'] = msg['band_flag'] == 1

  def mangle__commstate_flag(self, res, msg):
    res['cs'] = msg['commstate_flag'] == 1

  def mangle__display_flag(self, res, msg):
    res['display'] = msg['display_flag'] == 1

  def mangle__dsc_flag(self, res, msg):
    res['dsc'] = msg['dsc_flag'] == 1

  def mangle__m22_flag(self, res, msg):
    res['msg22'] = msg['m22_flag'] == 1

  def mangle__mode_flag(self, res, msg):
    res['mode'] = msg['mode_flag'] == 1

  def mangle__unit_flag(self, res, msg):
    res['unit'] = msg['unit_flag'] == 1

  # Type 19: Extended Class B CS Position Report

  def mangle__assigned_mode(self, res, msg):
    res['assigned'] = msg['assigned_mode'] == 1

  # Type 20 Data Link Management Message

  def mangle__reservations(self, res, msg):
    for idx, reservation in enumerate(msg['reservations']):
      i = str(idx + 1)
      res['increment' + i] = reservation['incr']
      res['number' + i] = reservation['num_slots']
      res['offset' + i] = reservation['offset']
      res['timeout' + i] = reservation['timeout']

  # Type 21: Aid-to-Navigation Report

  def mangle__aton_type(self, res, msg):
    res['aid_type'] = msg['aton_type']
    res['aid_type_text'] = self.aton_types[msg['aton_type']]

  def mangle__aton_status(self, res, msg):
    res['regional'] = msg['aton_status']

  def mangle__name__21(self, res, msg):
    res['name'] = msg['name'].strip('@').strip()

  def mangle__off_pos(self, res, msg):
    res['off_position'] = msg['off_pos']

  def mangle__virtual_aton(self, res, msg):
    res['virtual_aid'] = msg['virtual_aton']

  # Type 22: Channel Management

  def mangle__chan_a(self, res, msg):
    res['channel_a'] = msg['chan_a']

  def mangle__chan_b(self, res, msg):
    res['channel_b'] = msg['chan_b']

  def mangle__chan_a_bandwidth(self, res, msg):
    res['band_a'] = msg['chan_a_bandwidth']

  def mangle__chan_b_bandwidth(self, res, msg):
    res['band_b'] = msg['chan_b_bandwidth']

  def mangle__power_low(self, res, msg):
    res['power'] = msg['power_low']

  def mangle__txrx_mode(self, res, msg):
    res['txrx'] = msg['txrx_mode']

  def mangle__x1(self, res, msg):
    res['ne_lon'] = msg['x1']

  def mangle__x2(self, res, msg):
    res['sw_lon'] = msg['x2']

  def mangle__y2(self, res, msg):
    res['sw_lat'] = msg['y2']

  def mangle__y1(self, res, msg):
    res['ne_lat'] = msg['y1']

  def mangle__zone_size(self, res, msg):
    res['zonesize'] = msg['zone_size']

  # Type 23: Group Assignment Command

  def mangle__station_type(self, res, msg):
    res['stationtype'] = msg['station_type']
    res['stationtype_text'] = self.station_types[msg['station_type']]

  def mangle__interval_raw(self, res, msg):
    res['interval'] = msg['interval_raw']

  # Tagblock data
  def mangle__tagblock_timestamp(self, res, msg):
    res['tagblock_timestamp'] = datetime.datetime.utcfromtimestamp(
        msg['tagblock_timestamp']).strftime('%Y-%m-%dT%H:%M:%S.%fZ')

    if self.copy_tagblock_timestamp and 'year' not in msg:
      res['timestamp'] = res['tagblock_timestamp']

  # Mappings

  station_types = {
      0: 'All types of mobiles',
      1: 'Reserved for future use',
      2: 'All types of Class B mobile stations',
      3: 'SAR airborne mobile station',
      4: 'Aid to Navigation station',
      5: 'Class B shipborne mobile station (IEC62287 only)',
      6: 'Regional use and inland waterways',
      7: 'Regional use and inland waterways',
      8: 'Regional use and inland waterways',
      9: 'Regional use and inland waterways',
      10: 'Reserved for future use',
      11: 'Reserved for future use',
      12: 'Reserved for future use',
      13: 'Reserved for future use',
      14: 'Reserved for future use',
      15: 'Reserved for future use'}

  aton_types = {
      0: 'Default, Type of Aid to Navigation not specified',
      1: 'Reference point',
      2: 'RACON (radar transponder marking a navigation hazard)',
      3: 'Fixed structure off shore, such as oil platforms, wind farms, rigs.',
      4: 'Spare, Reserved for future use.',
      5: 'Light, without sectors',
      6: 'Light, with sectors',
      7: 'Leading Light Front',
      8: 'Leading Light Rear',
      9: 'Beacon, Cardinal N',
      10: 'Beacon, Cardinal E',
      11: 'Beacon, Cardinal S',
      12: 'Beacon, Cardinal W',
      13: 'Beacon, Port hand',
      14: 'Beacon, Starboard hand',
      15: 'Beacon, Preferred Channel port hand',
      16: 'Beacon, Preferred Channel starboard hand',
      17: 'Beacon, Isolated danger',
      18: 'Beacon, Safe water',
      19: 'Beacon, Special mark',
      20: 'Cardinal Mark N',
      21: 'Cardinal Mark E',
      22: 'Cardinal Mark S',
      23: 'Cardinal Mark W',
      24: 'Port hand Mark',
      25: 'Starboard hand Mark',
      26: 'Preferred Channel Port hand',
      27: 'Preferred Channel Starboard hand',
      28: 'Isolated danger',
      29: 'Safe Water',
      30: 'Special Mark',
      31: 'Light Vessel / LANBY / Rigs'}

  fix_types = {
      0: 'Undefined',
      1: 'GPS',
      2: 'GLONASS',
      3: 'Combined GPS/GLONASS',
      4: 'Loran-C',
      5: 'Chayka',
      6: 'Integrated navigation system',
      7: 'Surveyed',
      8: 'Galileo'}

  # Match the output of gpsd 3.11.
  nav_statuses = {
      0: 'Under way using engine',
      1: 'At anchor',
      2: 'Not under command',
      3: 'Restricted manoeuverability',  # Maneuverability.
      4: 'Constrained by her draught',
      5: 'Moored',
      6: 'Aground',
      7: 'Engaged in fishing',
      8: 'Under way sailing',
      # Reserved for future amendment of navigational status for ships
      # carrying DG, HS, or MP, or IMO hazard or pollutant category C,
      # high speed craft (HSC).
      9: 'Reserved for HSC',
      # Reserved for future amendment of navigational status for ships
      # carrying dangerous goods (DG), harmful substances (HS) or marine
      # pollutants (MP), or IMO hazard or pollutant category A, wing in
      # ground (WIG).
      10: 'Reserved for WIG',
      # Power-driven vessel towing astern (regional use).
      11: 'Reserved',
      # Power-driven vessel pushing ahead or towing alongside (regional use).
      12: 'Reserved',
      # Reserved for future use.
      13: 'Reserved',
      # AIS-SART (active), MOB-AIS, EPIRB-AIS,
      14: 'Reserved',
      # Default (also used by AIS-SART, MOB-AIS and EPIRB-AIS under test).
      15: 'Not defined'}

  ship_types = {
      0: 'Not available',
      1: 'Reserved for future use',
      2: 'Reserved for future use',
      3: 'Reserved for future use',
      4: 'Reserved for future use',
      5: 'Reserved for future use',
      6: 'Reserved for future use',
      7: 'Reserved for future use',
      8: 'Reserved for future use',
      9: 'Reserved for future use',
      10: 'Reserved for future use',
      11: 'Reserved for future use',
      12: 'Reserved for future use',
      13: 'Reserved for future use',
      14: 'Reserved for future use',
      15: 'Reserved for future use',
      16: 'Reserved for future use',
      17: 'Reserved for future use',
      18: 'Reserved for future use',
      19: 'Reserved for future use',
      20: 'Wing in ground (WIG), all ships of this type',
      21: 'Wing in ground (WIG), Hazardous category A',
      22: 'Wing in ground (WIG), Hazardous category B',
      23: 'Wing in ground (WIG), Hazardous category C',
      24: 'Wing in ground (WIG), Hazardous category D',
      25: 'Wing in ground (WIG), Reserved for future use',
      26: 'Wing in ground (WIG), Reserved for future use',
      27: 'Wing in ground (WIG), Reserved for future use',
      28: 'Wing in ground (WIG), Reserved for future use',
      29: 'Wing in ground (WIG), Reserved for future use',
      30: 'Fishing',
      31: 'Towing',
      32: 'Towing: length exceeds 200m or breadth exceeds 25m',
      33: 'Dredging or underwater ops',
      34: 'Diving ops',
      35: 'Military ops',
      36: 'Sailing',
      37: 'Pleasure Craft',
      38: 'Reserved',
      39: 'Reserved',
      40: 'High speed craft (HSC), all ships of this type',
      41: 'High speed craft (HSC), Hazardous category A',
      42: 'High speed craft (HSC), Hazardous category B',
      43: 'High speed craft (HSC), Hazardous category C',
      44: 'High speed craft (HSC), Hazardous category D',
      45: 'High speed craft (HSC), Reserved for future use',
      46: 'High speed craft (HSC), Reserved for future use',
      47: 'High speed craft (HSC), Reserved for future use',
      48: 'High speed craft (HSC), Reserved for future use',
      49: 'High speed craft (HSC), No additional information',
      50: 'Pilot Vessel',
      51: 'Search and Rescue vessel',
      52: 'Tug',
      53: 'Port Tender',
      54: 'Anti-pollution equipment',
      55: 'Law Enforcement',
      56: 'Spare - Local Vessel',
      57: 'Spare - Local Vessel',
      58: 'Medical Transport',
      59: 'Noncombatant ship according to RR Resolution No. 18',
      60: 'Passenger, all ships of this type',
      61: 'Passenger, Hazardous category A',
      62: 'Passenger, Hazardous category B',
      63: 'Passenger, Hazardous category C',
      64: 'Passenger, Hazardous category D',
      65: 'Passenger, Reserved for future use',
      66: 'Passenger, Reserved for future use',
      67: 'Passenger, Reserved for future use',
      68: 'Passenger, Reserved for future use',
      69: 'Passenger, No additional information',
      70: 'Cargo, all ships of this type',
      71: 'Cargo, Hazardous category A',
      72: 'Cargo, Hazardous category B',
      73: 'Cargo, Hazardous category C',
      74: 'Cargo, Hazardous category D',
      75: 'Cargo, Reserved for future use',
      76: 'Cargo, Reserved for future use',
      77: 'Cargo, Reserved for future use',
      78: 'Cargo, Reserved for future use',
      79: 'Cargo, No additional information',
      80: 'Tanker, all ships of this type',
      81: 'Tanker, Hazardous category A',
      82: 'Tanker, Hazardous category B',
      83: 'Tanker, Hazardous category C',
      84: 'Tanker, Hazardous category D',
      85: 'Tanker, Reserved for future use',
      86: 'Tanker, Reserved for future use',
      87: 'Tanker, Reserved for future use',
      88: 'Tanker, Reserved for future use',
      89: 'Tanker, No additional information',
      90: 'Other Type, all ships of this type',
      91: 'Other Type, Hazardous category A',
      92: 'Other Type, Hazardous category B',
      93: 'Other Type, Hazardous category C',
      94: 'Other Type, Hazardous category D',
      95: 'Other Type, Reserved for future use',
      96: 'Other Type, Reserved for future use',
      97: 'Other Type, Reserved for future use',
      98: 'Other Type, Reserved for future use',
      99: 'Other Type, no additional information'}

mangle = Mangler()
