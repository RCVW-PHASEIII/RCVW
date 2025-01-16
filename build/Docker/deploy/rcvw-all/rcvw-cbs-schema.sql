-- [rcvw-db].dbo.HRI_ACTIVATION_STATUS definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.HRI_ACTIVATION_STATUS;

CREATE TABLE [rcvw-db].dbo.HRI_ACTIVATION_STATUS (
	HRI_ID int NOT NULL,
	FixLatitude float NULL,
	FixLongitude float NULL,
	PreemptionStatus bit NULL,
	RBSOperational bit NULL,
	ErrorCode int NULL,
	ErrorMessage varchar(1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	RTCMProxy bit DEFAULT 0 NULL,
	IP varchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	LastUpdated datetime DEFAULT getdate() NOT NULL,
	CONSTRAINT PK_HRI_ACTIVATION_STATUS PRIMARY KEY (HRI_ID)
);


-- [rcvw-db].dbo.HRI_CROSSING_HEADER definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.HRI_CROSSING_HEADER;

CREATE TABLE [rcvw-db].dbo.HRI_CROSSING_HEADER (
	HRI_ID int NULL,
	HRIProtWarnDevicePresent bit NULL,
	AgencyID int NULL,
	CountyCode nvarchar(5) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	CrossingID nvarchar(7) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	PublishedReportBaseID int NULL,
	StateCode nvarchar(2) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	RailroadCode nvarchar(8) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	CrossingClosed bit NULL,
	Created datetime NULL,
	CreatedBy nvarchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	LastUpdated datetime NULL,
	LastUpdateBy nvarchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	CONSTRAINT PK_HRI_CROSSING_HEADER PRIMARY KEY (CrossingID)
);

-- [rcvw-db].dbo.HRI_HIGHWAY_TRAFFIC_CONTROL_DEVICE definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.HRI_HIGHWAY_TRAFFIC_CONTROL_DEVICE;

CREATE TABLE [rcvw-db].dbo.HRI_HIGHWAY_TRAFFIC_CONTROL_DEVICE (
	CrossingID nvarchar(7) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	SubmissionType nvarchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	ReportBaseId int NOT NULL,
	NoSigns nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	XBuck int NULL,
	StopStd int NULL,
	YieldStd int NULL,
	AdvWarn nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	AdvW10_1 nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	AdvW10_2 nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	AdvW10_3 nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	AdvW10_4 nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	AdvW10_11 nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	AdvW10_12 nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Low_Grnd nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Low_GrndSigns int NULL,
	PaveMrkIDs nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Channel nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Exempt nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	EnsSign nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	OthSgn nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	OthSgn1 int NULL,
	OthDes1 nvarchar(10) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	OthSgn2 int NULL,
	OthDes2 nvarchar(10) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	OthSgn3 int NULL,
	OthDes3 nvarchar(10) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	PrvxSign nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Led nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Gates int NULL,
	GatePed int NULL,
	GateConf nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	GateConfType nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	FlashOv int NULL,
	FlashNov int NULL,
	CFlashType nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	FlashPost int NULL,
	FlashPostType nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Bkl_FlashPost nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Sdl_FlashPost nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	FlashPai int NULL,
	AwdIDate nvarchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	AwhornChk nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	AwhornlDate nvarchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	HwyTrafSignl nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Wigwags tinyint NULL,
	Bells int NULL,
	SpecPro nvarchar(20) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	FlashOth int NULL,
	FlashOthDes nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	HwynrSig nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Intrprmp nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	PrempType nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	HwtrfPsig nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	HwtrfPsigsdis int NULL,
	HwtrfPsiglndis int NULL,
	MonitorDev nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	WdCode varchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	CONSTRAINT PK_HRI_HIGHWAY_TRAFFIC_CONTROL_DEVICE PRIMARY KEY (CrossingID),
	CONSTRAINT FK_HRI_HIGHWAY_TRAFFIC_CONTROL_DEVICE_HRI_CROSSING_HEADER FOREIGN KEY (CrossingID) REFERENCES [rcvw-db].dbo.HRI_CROSSING_HEADER(CrossingID) ON DELETE CASCADE
);


-- [rcvw-db].dbo.HRI_LOCATION_AND_CLASSIFICATION definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.HRI_LOCATION_AND_CLASSIFICATION;

CREATE TABLE [rcvw-db].dbo.HRI_LOCATION_AND_CLASSIFICATION (
	CrossingID nvarchar(7) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	SubmissionType nvarchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	ReportBaseId int NOT NULL,
	Railroad nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	StateCD nvarchar(2) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	StateName nvarchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	CntyCD nvarchar(10) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	CountyName nvarchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Nearest nvarchar(6) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	CityCD nvarchar(10) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	CityName nvarchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Street nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	BlockNumb nvarchar(6) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Highway nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	SepInd nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	SepRr1 nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	SepRr2 nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	SepRr3 nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	SepRr4 nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	MultFrmsFiled nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	SameInd nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	SameRr1 nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	SameRr2 nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	SameRr3 nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	SameRr4 nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	RrID nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Ttstn nvarchar(6) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	TtstnNam nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	RrMain nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	XingOwnr nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	TypeXing nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	XPurpose nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	PosXing nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	OpenPub nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	TypeTrnSrvcIDs nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	DevelTypID nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	XingAdj nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	XngAdjNo nvarchar(7) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Whistban nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	WhistDate nvarchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	HscoRrid nvarchar(4) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	SfxHscoRrid nvarchar(4) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Latitude float NULL,
	Longitude float NULL,
	LLsource nvarchar(10) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	RrNarr1 nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	RrNarr2 nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	RrNarr3 nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	RrNarr4 nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	StNarr1 nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	StNarr2 nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	StNarr3 nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	StNarr4 nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	RrNarr nvarchar(MAX) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	StNarr nvarchar(MAX) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	PolCont nvarchar(10) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	RrCont nvarchar(10) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	HwyCont nvarchar(10) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	CONSTRAINT PK_HRI_LOCATION_AND_CLASSIFICATION PRIMARY KEY (CrossingID),
	CONSTRAINT FK_HRI_LOCATION_AND_CLASSIFICATION_HRI_CROSSING_HEADER FOREIGN KEY (CrossingID) REFERENCES [rcvw-db].dbo.HRI_CROSSING_HEADER(CrossingID)
);


-- [rcvw-db].dbo.HRI_PHYSICAL_CHARACTERISTICS definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.HRI_PHYSICAL_CHARACTERISTICS;

CREATE TABLE [rcvw-db].dbo.HRI_PHYSICAL_CHARACTERISTICS (
	CrossingID nvarchar(7) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	SubmissionType nvarchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	ReportBaseId int NOT NULL,
	TraficLn int NULL,
	TraflnType int NULL,
	HwyPved nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Downst nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Illumina nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	XSurfDate nvarchar(6) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	XSurfWidth int NULL,
	XSurfLength int NULL,
	XSurfaceIDs nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	XSurOthr nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	HwyNear nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	HwynDist int NULL,
	XAngle nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	ComPower nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	CONSTRAINT PK_HRI_PHYSICAL_CHARACTERISTICS PRIMARY KEY (CrossingID),
	CONSTRAINT FK_HRI_PHYSICAL_CHARACTERISTICS_HRI_CROSSING_HEADER FOREIGN KEY (CrossingID) REFERENCES [rcvw-db].dbo.HRI_CROSSING_HEADER(CrossingID)
);


-- [rcvw-db].dbo.HRI_PUBLIC_HIGHWAY definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.HRI_PUBLIC_HIGHWAY;

CREATE TABLE [rcvw-db].dbo.HRI_PUBLIC_HIGHWAY (
	CrossingID nvarchar(7) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	SubmissionType nvarchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	ReportBaseId int NOT NULL,
	HwySys nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	HwyClassCD nvarchar(10) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	HwyClassrdtpID nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	StHwy1 nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	HwySpeed int NULL,
	HwySpeedps nvarchar(32) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	LrsRouteid nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	LrsMilePost nvarchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Aadt float NULL,
	AadtYear nvarchar(4) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	PctTruk float NULL,
	SchlBusChk nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	SchlBsCnt float NULL,
	HazmtVeh nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	EmrgncySrvc nvarchar(1) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	crossingClosed nvarchar(3) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	CONSTRAINT PK_HRI_PUBLIC_HIGHWAY PRIMARY KEY (CrossingID),
	CONSTRAINT FK_HRI_PUBLIC_HIGHWAY_HRI_CROSSING_HEADER FOREIGN KEY (CrossingID) REFERENCES [rcvw-db].dbo.HRI_CROSSING_HEADER(CrossingID)
);


-- [rcvw-db].dbo.HRI_CBS_CONNECT definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.HRI_CBS_CONNECT;

CREATE TABLE [rcvw-db].dbo.HRI_CBS_CONNECT (
	HRI_ID int NOT NULL,
	[Type] varchar(128) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	Endpoint varchar(MAX) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Topic varchar(512) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Subscription varchar(512) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	CONSTRAINT PK_HRI_CBS_CONNECT PRIMARY KEY ([Type],HRI_ID),
	CONSTRAINT FK_HRI_CBS_CONNECT_HRI_ACTIVATION_STATUS FOREIGN KEY (HRI_ID) REFERENCES [rcvw-db].dbo.HRI_ACTIVATION_STATUS(HRI_ID)
);


-- [rcvw-db].dbo.RBS_INCOMING_MESSAGE definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.RBS_INCOMING_MESSAGE;

CREATE TABLE [rcvw-db].dbo.RBS_INCOMING_MESSAGE (
	Message_ID uniqueidentifier NOT NULL,
	Topic varchar(512) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Subject varchar(512) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Content_Type varchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Content varchar(MAX) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	Received datetime NOT NULL,
	Stored datetime NOT NULL,
	CONSTRAINT PK_RBS_INCOMING_MESSAGE PRIMARY KEY (Message_ID)
);
 CREATE NONCLUSTERED INDEX receivedIndex ON dbo.RBS_INCOMING_MESSAGE (  Received ASC  )  
	 WITH (  PAD_INDEX = OFF ,FILLFACTOR = 100  ,SORT_IN_TEMPDB = OFF , IGNORE_DUP_KEY = OFF , STATISTICS_NORECOMPUTE = OFF , ONLINE = OFF , ALLOW_ROW_LOCKS = ON , ALLOW_PAGE_LOCKS = ON  )
	 ON [PRIMARY ] ;


-- [rcvw-db].dbo.RBS_IEEE1570_4909_MESSAGE definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.RBS_IEEE1570_4909_MESSAGE;

CREATE TABLE [rcvw-db].dbo.RBS_IEEE1570_4909_MESSAGE (
	Message_ID uniqueidentifier NOT NULL,
	HRI_ID int NOT NULL,
	IEEE1570 bit NOT NULL,
	xingSequence int NULL,
	xingNumTracks int NULL,
	xingNumTrains bit NULL,
	xingOperational bit NULL,
	xingTrainDetected bit NULL,
	xingWarningActive bit NULL,
	xingPreemptionActive bit NULL,
	xingEgressGates bit NULL,
	xingEgressGatesUp bit NULL,
	xingEgressGatesDown bit NULL,
	xingIngressGates bit NULL,
	xingIngressGatesUp bit NULL,
	xingIngressGatesDown bit NULL,
	xingPreemptionDesignSeconds int NULL,
	trainSequence int NULL,
	trainDirection text COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	trainIslandOccupied bit NULL,
	trainWarningDesignTime int NULL,
	trainEstimatedWarningSeconds int NULL,
	CONSTRAINT FK_RBS_IEEE1509_4909_MESSAGE_RBS_INCOMING_MESSAGE FOREIGN KEY (Message_ID) REFERENCES [rcvw-db].dbo.RBS_INCOMING_MESSAGE(Message_ID),
	CONSTRAINT FK_RBS_INCOMING_1509_CROSSINGS_HRI_ACTIVATION_STATUS FOREIGN KEY (HRI_ID) REFERENCES [rcvw-db].dbo.HRI_ACTIVATION_STATUS(HRI_ID)
);


-- [rcvw-db].dbo.RBS_INCOMING_MAP definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.RBS_INCOMING_MAP;

CREATE TABLE [rcvw-db].dbo.RBS_INCOMING_MAP (
	Message_ID uniqueidentifier NOT NULL,
	HRI_ID int NOT NULL,
	TrackedLane int NOT NULL,
	SignalGroup int NOT NULL,
	RefPointLatitude float NOT NULL,
	RefPointLongitude float NOT NULL,
	Received datetime DEFAULT getdate() NULL,
	CONSTRAINT FK_RBS_INCOMING_MAP_HRI_ACTIVATION_STATUS FOREIGN KEY (HRI_ID) REFERENCES [rcvw-db].dbo.HRI_ACTIVATION_STATUS(HRI_ID),
	CONSTRAINT FK_RBS_INCOMING_MAP_RBS_INCOMING_MESSAGE FOREIGN KEY (Message_ID) REFERENCES [rcvw-db].dbo.RBS_INCOMING_MESSAGE(Message_ID)
);
 CREATE NONCLUSTERED INDEX nci_msft_1_RBS_INCOMING_MAP_2F7F6C58CFE38F9CDA79D649EC2F9189 ON dbo.RBS_INCOMING_MAP (  HRI_ID ASC  , Received ASC  )  
	 INCLUDE ( SignalGroup ) 
	 WITH (  PAD_INDEX = OFF ,FILLFACTOR = 100  ,SORT_IN_TEMPDB = OFF , IGNORE_DUP_KEY = OFF , STATISTICS_NORECOMPUTE = OFF , ONLINE = OFF , ALLOW_ROW_LOCKS = ON , ALLOW_PAGE_LOCKS = ON  )
	 ON [PRIMARY ] ;


-- [rcvw-db].dbo.RBS_INCOMING_SPAT definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.RBS_INCOMING_SPAT;

CREATE TABLE [rcvw-db].dbo.RBS_INCOMING_SPAT (
	Message_ID uniqueidentifier NOT NULL,
	HRI_ID int NOT NULL,
	IntersectionName varchar(50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	MsgTime datetime NOT NULL,
	ActiveSignalGroup int NOT NULL,
	CONSTRAINT FK_RBS_INCOMING_SPAT_HRI_ACTIVATION_STATUS FOREIGN KEY (HRI_ID) REFERENCES [rcvw-db].dbo.HRI_ACTIVATION_STATUS(HRI_ID),
	CONSTRAINT FK_RBS_INCOMING_SPAT_RBS_INCOMING_MESSAGE FOREIGN KEY (Message_ID) REFERENCES [rcvw-db].dbo.RBS_INCOMING_MESSAGE(Message_ID)
);
 CREATE NONCLUSTERED INDEX Index_RBS_INCOMING_SPAT_1 ON dbo.RBS_INCOMING_SPAT (  HRI_ID ASC  , MsgTime ASC  )  
	 WITH (  PAD_INDEX = OFF ,FILLFACTOR = 100  ,SORT_IN_TEMPDB = OFF , IGNORE_DUP_KEY = OFF , STATISTICS_NORECOMPUTE = OFF , ONLINE = OFF , ALLOW_ROW_LOCKS = ON , ALLOW_PAGE_LOCKS = ON  )
	 ON [PRIMARY ] ;

-- [rcvw-db].dbo.RBS_RADIOS definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.RBS_RADIOS;

CREATE TABLE [rcvw-db].dbo.RBS_RADIOS (
	HRI_ID int NOT NULL,
	Make varchar(256) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	Version varchar(1024) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	Location varchar(1024) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	Firmware varchar(1024) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	Power int NOT NULL,
	Mode varchar(10) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	Sent int NOT NULL,
	Received int NOT NULL
);


-- [rcvw-db].dbo.RCVW_CLIENTS definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.RCVW_CLIENTS;

CREATE TABLE [rcvw-db].dbo.RCVW_CLIENTS (
	UUID varchar(64) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	ENTITY_NAME varchar(64) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	LAST_QUERY_DATE datetime DEFAULT getdate() NOT NULL,
	CONSTRAINT PK__RCVW_CLI__65A475E7D471BCC1 PRIMARY KEY (UUID)
);

-- [rcvw-db].dbo.RSU_GPS_OUTPUT definition

-- Drop table

-- DROP TABLE [rcvw-db].dbo.RSU_GPS_OUTPUT;

CREATE TABLE [rcvw-db].dbo.RSU_GPS_OUTPUT (
	HRI_ID int NOT NULL,
	rsuGpsOutputPort int NULL,
	rsuGpsOutputAddress varchar(40) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	rsuOutputInterface varchar(10) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	rsuGpsOutputInterval int NULL,
	rsuGpsOutputString varchar(200) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	rsuGpsRefLat int NULL,
	rsuGpsRefLon int NULL,
	rsuGpsRefElv int NULL,
	rsuGpsMaxDeviation int NULL,
	CONSTRAINT FK_RSU_GPS_OUTPUT_HRI_ACTIVATION_STATUS FOREIGN KEY (HRI_ID) REFERENCES [rcvw-db].dbo.HRI_ACTIVATION_STATUS(HRI_ID)
);

-- dbo.HRI_CROSSINGS source

CREATE VIEW [dbo].[HRI_CROSSINGS]
AS
SELECT        A.HRI_ID, A.HRIProtWarnDevicePresent, A.AgencyID, A.CountyCode, A.PublishedReportBaseID, A.StateCode, A.RailroadCode, A.Created, A.CreatedBy, A.LastUpdated, A.LastUpdateBy, B.AdvWarn, B.AwdIDate, B.AwhornChk, 
                         B.AwhornlDate, B.Bells, B.Bkl_FlashPost, B.CFlashType, B.Channel, B.EnsSign, B.Exempt, B.FlashNov, B.FlashOth, B.FlashOthDes, B.FlashOv, B.FlashPai, B.FlashPostType, B.GateConf, B.GateConfType, B.GatePed, B.Gates, 
                         B.HwtrfPsig, B.HwtrfPsiglndis, B.HwtrfPsigsdis, B.HwynrSig, B.HwyTrafSignl, B.Intrprmp, B.Led, B.Low_Grnd, B.Low_GrndSigns, B.MonitorDev, B.NoSigns, B.OthDes1, B.OthDes2, B.OthDes3, B.OthSgn, B.OthSgn1, B.OthSgn2, 
                         B.OthSgn3, B.PaveMrkIDs, B.PrempType, B.PrvxSign, B.ReportBaseId, B.Sdl_FlashPost, B.SpecPro, B.StopStd, B.XBuck, B.YieldStd, B.WdCode, C.BlockNumb, C.CityCD, C.CntyCD, C.DevelTypID, C.Highway, C.HscoRrid, 
                         C.HwyCont, C.Latitude, C.LLsource, C.Longitude, C.MultFrmsFiled, C.Nearest, C.OpenPub, C.PolCont, C.PosXing, C.Railroad, C.RrCont, C.RrID, C.RrMain, C.RrNarr, C.RrNarr1, C.RrNarr2, C.RrNarr3, C.RrNarr4, C.SameInd, 
                         C.SameRr1, C.SameRr2, C.SameRr3, C.SameRr4, C.SepInd, C.SepRr1, C.SepRr2, C.SepRr3, C.SepRr4, C.SfxHscoRrid, C.StateCD, C.StNarr, C.StNarr1, C.StNarr2, C.StNarr3, C.StNarr4, C.Street, C.Ttstn, C.TtstnNam, 
                         C.TypeTrnSrvcIDs, C.TypeXing, C.Whistban, C.WhistDate, C.XingAdj, C.XingOwnr, C.XngAdjNo, C.XPurpose, D.ComPower, D.Downst, D.HwynDist, D.HwyNear, D.HwyPved, D.Illumina, D.TraficLn, D.TraflnType, D.XAngle, 
                         D.XSurfaceIDs, D.XSurfDate, D.XSurfLength, D.XSurfWidth, D.XSurOthr, E.Aadt, E.AadtYear, E.EmrgncySrvc, E.HwyClassCD, E.HwyClassrdtpID, E.HwySpeed, E.HwySpeedps, E.HwySys, E.LrsMilePost, E.LrsRouteid, E.PctTruk, 
                         E.SchlBsCnt, E.SchlBusChk, E.StHwy1, C.StateName, C.CountyName, C.CityName, B.CrossingID, B.AdvW10_2, B.AdvW10_1, B.AdvW10_3, B.AdvW10_4, B.AdvW10_11, B.AdvW10_12, B.FlashPost, B.Wigwags, E.HazmtVeh, 
                         E.crossingClosed, dbo.HRI_ACTIVATION_STATUS.FixLatitude, dbo.HRI_ACTIVATION_STATUS.FixLongitude, dbo.HRI_ACTIVATION_STATUS.RTCMProxy, dbo.HRI_ACTIVATION_STATUS.PreemptionStatus, dbo.HRI_ACTIVATION_STATUS.RBSOperational, 
                         dbo.HRI_ACTIVATION_STATUS.ErrorCode, dbo.HRI_ACTIVATION_STATUS.ErrorMessage, dbo.HRI_ACTIVATION_STATUS.IP, dbo.HRI_CBS_CONNECT.Endpoint, dbo.HRI_CBS_CONNECT.Topic, dbo.HRI_CBS_CONNECT.Subscription
FROM            dbo.HRI_CROSSING_HEADER AS A INNER JOIN
                         dbo.HRI_HIGHWAY_TRAFFIC_CONTROL_DEVICE AS B ON A.CrossingID = B.CrossingID INNER JOIN
                         dbo.HRI_LOCATION_AND_CLASSIFICATION AS C ON A.CrossingID = C.CrossingID INNER JOIN
                         dbo.HRI_PHYSICAL_CHARACTERISTICS AS D ON A.CrossingID = D.CrossingID INNER JOIN
                         dbo.HRI_PUBLIC_HIGHWAY AS E ON A.CrossingID = E.CrossingID LEFT OUTER JOIN
                         dbo.HRI_ACTIVATION_STATUS ON A.HRI_ID = dbo.HRI_ACTIVATION_STATUS.HRI_ID LEFT OUTER JOIN
                         dbo.HRI_CBS_CONNECT ON A.HRI_ID = dbo.HRI_CBS_CONNECT.HRI_ID AND dbo.HRI_CBS_CONNECT.Type = 'Events';


-- dbo.RBS_INCOMING_MAP_RATE source

CREATE VIEW [dbo].[RBS_INCOMING_MAP_RATE]
AS
SELECT HRI.HRI_ID, 
        COALESCE(DATEDIFF(SECOND, MIN(Received), MAX(Received)), 0) AS SecondCount, 
        SUM(CASE WHEN MAP.HRI_ID IS NULL THEN 0 ELSE 1 END) AS TotalMsgs, 
        CASE WHEN DATEDIFF(SECOND, MIN(Received), MAX(Received)) > 0 THEN 1.0 * COUNT(*) / DATEDIFF(SECOND, MIN(Received), MAX(Received)) ELSE 0.0 END AS MsgRate
FROM [dbo].[HRI_ACTIVATION_STATUS] HRI
LEFT JOIN [dbo].[RBS_INCOMING_MAP] MAP
ON HRI.HRI_ID = MAP.HRI_ID 
AND MAP.Received > HRI.LastUpdated 
AND MAP.Received > DATEADD(SECOND, -10, GETDATE())
GROUP BY HRI.HRI_ID;


-- dbo.RBS_INCOMING_MESSAGE_RATE source

CREATE VIEW [dbo].[RBS_INCOMING_MESSAGE_RATE]
AS
SELECT HRI.HRI_ID, MSG.Topic, 
        COALESCE(DATEDIFF(SECOND, MIN(Received), MAX(Received)), 0) AS SecondCount, 
        SUM(CASE WHEN MSG.Received IS NULL THEN 0 ELSE 1 END) AS TotalMsgs,
        CASE WHEN DATEDIFF(SECOND, MIN(Received), MAX(Received)) > 0 THEN 1.0 * COUNT(*) / DATEDIFF(SECOND, MIN(Received), MAX(Received)) ELSE 0.0 END AS MsgRate
FROM [dbo].[HRI_ACTIVATION_STATUS] HRI
LEFT JOIN [dbo].[RBS_INCOMING_MESSAGE] MSG
ON HRI.HRI_ID = CASE WHEN Subject IS NOT NULL 
AND LEN(Subject) > 0
THEN CAST(RIGHT(LEFT(Subject, 14), 5) AS INT) 
ELSE 0 END
AND MSG.Topic LIKE 'tmx.plugin.%.status'
AND MSG.Received > DATEADD(SECOND, -10, GETDATE())
GROUP BY HRI.HRI_ID, MSG.Topic;


-- dbo.RBS_INCOMING_SPAT_PERFORMANCE source

CREATE VIEW [dbo].[RBS_INCOMING_SPAT_PERFORMANCE]
AS
SELECT MSG.Message_ID, SPAT.HRI_ID, MSG.Content, SPAT.IntersectionName, SPAT.ActiveSignalGroup, SPAT.MsgTime, MSG.Received, MSG.Stored,
DATEDIFF(MILLISECOND, SPAT.MsgTime, MSG.Received) AS ReceivedIn_msec, 
DATEDIFF(MILLISECOND, SPAT.MsgTime, MSG.Stored) AS StoredIn_msec
FROM dbo.RBS_INCOMING_SPAT SPAT
JOIN dbo.RBS_INCOMING_MESSAGE MSG
ON SPAT.Message_ID = MSG.Message_ID AND SPAT.HRI_ID = MSG.Subject AND MSG.Content_Type = 'SPAT';


-- dbo.RBS_INCOMING_SPAT_RATE source

CREATE VIEW [dbo].[RBS_INCOMING_SPAT_RATE]
AS
SELECT HRI.HRI_ID, 
        COALESCE(DATEDIFF(SECOND, MIN(MsgTime), MAX(MsgTime)), 0) AS SecondCount, 
        SUM(CASE WHEN SPAT.HRI_ID IS NULL THEN 0 ELSE 1 END) AS TotalMsgs, 
        CASE WHEN DATEDIFF(SECOND, MIN(MsgTime), MAX(MsgTime)) > 0 THEN 1.0 * COUNT(*) / DATEDIFF(SECOND, MIN(MsgTime), MAX(MsgTime)) ELSE 0.0 END AS MsgRate
FROM [dbo].[HRI_ACTIVATION_STATUS] HRI
LEFT JOIN [dbo].[RBS_INCOMING_SPAT] SPAT
ON HRI.HRI_ID = SPAT.HRI_ID
AND SPAT.MsgTime > DATEADD(SECOND, -10, GETDATE())
GROUP BY HRI.HRI_ID;


-- dbo.RBS_INCOMING_SPAT_STATUS source

CREATE VIEW [dbo].[RBS_INCOMING_SPAT_STATUS]
AS
SELECT HRI.HRI_ID, SPAT.Message_ID AS SPATMsgId, SPAT.MsgTime AS SPATMsgTime, 
    MAP.Message_ID AS MAPMsgId, MAP.Received AS MAPMsgTime,
    MAP.RefPointLatitude, MAP.RefPointLongitude, 
    SPAT.IntersectionName, SPAT.ActiveSignalGroup, MAP.TrackedLane,
    CASE WHEN SPAT.ActiveSignalGroup = MAP.SignalGroup THEN 1 ELSE 0 END AS HRIActive
FROM [dbo].[HRI_ACTIVATION_STATUS] HRI
LEFT JOIN [dbo].[RBS_LAST_SPAT] LSPAT 
ON LSPAT.HRI_ID = HRI.HRI_ID
LEFT JOIN [dbo].[RBS_LAST_MAP] LMAP
ON LMAP.HRI_ID = HRI.HRI_ID
LEFT JOIN [dbo].[RBS_INCOMING_SPAT] SPAT
ON SPAT.HRI_ID = HRI.HRI_ID 
AND SPAT.MsgTime = LSPAT.LastSpat
LEFT JOIN [dbo].[RBS_INCOMING_MAP] MAP
ON MAP.HRI_ID = HRI.HRI_ID
AND MAP.Received = LMAP.LastMap;


-- dbo.RBS_LAST_MAP source

CREATE VIEW RBS_LAST_MAP
AS
SELECT HRI_ID, MAX(Received) as LastMap
FROM RBS_INCOMING_MAP
GROUP BY HRI_ID;


-- dbo.RBS_LAST_SPAT source

CREATE VIEW RBS_LAST_SPAT
AS
SELECT HRI_ID, MAX(MsgTime) as LastSpat
FROM RBS_INCOMING_SPAT
GROUP BY HRI_ID;
