#!/usr/bin/python3

configs = [ 
    
                {   # Config for TSCH (minmal schedule, 3 slots)
                    "name": "TSCH-minimal-3", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 1), 
                            ("USE_ORCHESTRA", 0), 
                            ("USE_RADIO_ENCRYPTION", 1),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 0),
                            ("USE_SECURITY", 0),
                            ("USE_STATISTICS", 0),
                            ("TSCH_SCHEDULE_CONF_DEFAULT_LENGTH", 3)
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": True },
                    "benchmark-conf": # Just make sure that restart is disabled
                        [   ("ROOT_RESTART_INTERVAL" , "(0 * CLOCK_SECOND)"),
                            ("NODE_RESTART_INTERVAL", "(0 * CLOCK_SECOND)"),
                            ("USE_LATENCY_MEASUREMENT", 1)
                        ]
                },
                {   # Config for TSCH (minmal schedule, 6 slots)
                    "name": "TSCH-minimal-6", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 1), 
                            ("USE_ORCHESTRA", 0), 
                            ("USE_RADIO_ENCRYPTION", 1),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 0),
                            ("USE_SECURITY", 0),
                            ("USE_STATISTICS", 0),
                            ("TSCH_SCHEDULE_CONF_DEFAULT_LENGTH", 6)
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": True },
                },
                {   # Config for TSCH (minimal schedule, 12 slots)
                    "name": "TSCH-minimal-12", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 1), 
                            ("USE_ORCHESTRA", 0), 
                            ("USE_RADIO_ENCRYPTION", 1),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 0),
                            ("USE_SECURITY", 1),
                            ("USE_STATISTICS", 0),
                            ("TSCH_SCHEDULE_CONF_DEFAULT_LENGTH", 12)
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": True }
                },
                {   # Config for TSCH (minmal schedule, 20 slots)
                    "name": "TSCH-minimal-25", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 1), 
                            ("USE_ORCHESTRA", 0), 
                            ("USE_RADIO_ENCRYPTION", 1),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 0),
                            ("USE_SECURITY", 0),
                            ("USE_STATISTICS", 0),
                            ("TSCH_SCHEDULE_CONF_DEFAULT_LENGTH", 25)
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": True }
                },
                {   # Config for TSCH (orchestra default config)
                    "name": "TSCH-orchestra-RB-7", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 1), 
                            ("USE_ORCHESTRA", 1), 
                            ("USE_RADIO_ENCRYPTION", 1),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 0),
                            ("USE_SECURITY", 0),
                            ("USE_STATISTICS", 0),
                            ("TSCH_SCHEDULE_CONF_DEFAULT_LENGTH", 25),
                            ("ORCHESTRA_CONF_UNICAST_SENDER_BASED", 0),
                            ("ORCHESTRA_CONF_EBSF_PERIOD", 53),
                            ("ORCHESTRA_CONF_COMMON_SHARED_PERIOD", 17),
                            ("ORCHESTRA_CONF_UNICAST_PERIOD", 7),
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": True }
                },
                {   # Config for TSCH (orchestra default config)
                    "name": "TSCH-orchestra-SB-7", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 1), 
                            ("USE_ORCHESTRA", 1), 
                            ("USE_RADIO_ENCRYPTION", 1),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 0),
                            ("USE_SECURITY", 0),
                            ("USE_STATISTICS", 0),
                            ("TSCH_SCHEDULE_CONF_DEFAULT_LENGTH", 25),
                            ("ORCHESTRA_CONF_UNICAST_SENDER_BASED", 1),
                            ("ORCHESTRA_CONF_EBSF_PERIOD", 53),
                            ("ORCHESTRA_CONF_COMMON_SHARED_PERIOD", 17),
                            ("ORCHESTRA_CONF_UNICAST_PERIOD", 7),
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": True }
                },
                {   # Config for ContikiMAC (2 Hz)
                    "name": "ContikiMAC-2", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 0), 
                            ("USE_ORCHESTRA", 0), 
                            ("USE_RADIO_ENCRYPTION", 0),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 1),
                            ("USE_SECURITY", 1),
                            ("USE_STATISTICS", 1),
                            ("NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE", 2)
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": False }
                },
                {   # Config for ContikiMAC (4 Hz)
                    "name": "ContikiMAC-4", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 0), 
                            ("USE_ORCHESTRA", 0), 
                            ("USE_RADIO_ENCRYPTION", 0),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 1),
                            ("USE_SECURITY", 1),
                            ("USE_STATISTICS", 1),
                            ("NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE", 4)
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": False }
                },
                {   # Config for ContikiMAC (8 Hz)
                    "name": "ContikiMAC-8", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 0), 
                            ("USE_ORCHESTRA", 0), 
                            ("USE_RADIO_ENCRYPTION", 0),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 1),
                            ("USE_SECURITY", 1),
                            ("USE_STATISTICS", 1),
                            ("NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE", 8)
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": False }
                },
                {   # Config for ContikiMAC (16 Hz)
                    "name": "ContikiMAC-16", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 0), 
                            ("USE_ORCHESTRA", 0), 
                            ("USE_RADIO_ENCRYPTION", 0),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 1),
                            ("USE_SECURITY", 1),
                            ("USE_STATISTICS", 1),
                            ("NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE", 16)
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": False }
                },
                {   # Config for ContikiMAC (8 Hz)
                    "name": "ContikiMAC-8-Reboot-Root", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 0), 
                            ("USE_ORCHESTRA", 0), 
                            ("USE_RADIO_ENCRYPTION", 0),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 1),
                            ("USE_SECURITY", 1),
                            ("USE_STATISTICS", 1),
                            ("NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE", 8)
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": False },
                    "benchmark-conf":
                        [   
                            ("ROOT_RESTART_INTERVAL" , "(30 * 60 * CLOCK_SECOND)"),
                            ("NODE_RESTART_INTERVAL" , "(0 * CLOCK_SECOND)"), 
                        ]
                },
                {   # Config for ContikiMAC (8 Hz)
                    "name": "ContikiMAC-8-Reboot-Node", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 0), 
                            ("USE_ORCHESTRA", 0), 
                            ("USE_RADIO_ENCRYPTION", 0),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 1),
                            ("USE_SECURITY", 1),
                            ("USE_STATISTICS", 1),
                            ("NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE", 8)
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": False },
                    "benchmark-conf":
                        [   
                            ("ROOT_RESTART_INTERVAL" , "(0 * CLOCK_SECOND)"),
                            ("NODE_RESTART_INTERVAL" , "(30 * 60 * CLOCK_SECOND)"), 
                        ]
                },
                {   # Config for TSCH (minmal schedule, 6 slots)
                    "name": "TSCH-minimal-6-Reboot-Root", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 1), 
                            ("USE_ORCHESTRA", 0), 
                            ("USE_RADIO_ENCRYPTION", 1),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 0),
                            ("USE_SECURITY", 0),
                            ("USE_STATISTICS", 0),
                            ("TSCH_SCHEDULE_CONF_DEFAULT_LENGTH", 6)
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": True },
                    "benchmark-conf":
                        [   
                            ("ROOT_RESTART_INTERVAL" , "(30 * 60 * CLOCK_SECOND)"),
                            ("NODE_RESTART_INTERVAL" , "(0 * CLOCK_SECOND)"), 
                        ]
                },
                {   # Config for TSCH (minmal schedule, 6 slots)
                    "name": "TSCH-minimal-6-Reboot-Node", 
                    "contiki-conf": 
                        [   
                            ("USE_TSCH", 1), 
                            ("USE_ORCHESTRA", 0), 
                            ("USE_RADIO_ENCRYPTION", 1),
                            ("TSCH_DEBUG", 0),
                            ("USE_RDC", 0),
                            ("USE_SECURITY", 0),
                            ("USE_STATISTICS", 0),
                            ("TSCH_SCHEDULE_CONF_DEFAULT_LENGTH", 6)
                        ],
                    "Makefile":
                        { "ENABLE_TSCH": True },
                    "benchmark-conf":
                        [   
                            ("ROOT_RESTART_INTERVAL" , "(0 * CLOCK_SECOND)"),
                            ("NODE_RESTART_INTERVAL" , "(30 * 60 * CLOCK_SECOND)"), 
                        ]
                },
            ]