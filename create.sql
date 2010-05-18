CREATE TABLE vessel_pos (
       mmsi INTEGER PRIMARY KEY,
       cog INTEGER,
       sog REAL,
       time_stamp TIMESTAMP WITH TIME ZONE DEFAULT now(),
       nav_status VARCHAR(30), -- convert the code to a string for presentation
       );
SELECT AddGeometryColumn('vessel_pos','pos',4326,'POINT',2);
SELECT AddGeometryColumn('vessel_pos','track',4326,'LINESTRING',2);

CREATE TABLE vessel_name (
       mmsi INTEGER PRIMARY KEY,
       name VARCHAR(25), -- only need 20
       type_and_cargo INTEGER,
       response_class INTEGER -- 0 do not display differently.  1 == response vessel
);
