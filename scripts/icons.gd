class_name Icons
extends Node

const ICONS = [
    {
        "extensions": ["rs"],
        "icon": "",
    },
    {
        "extensions": ["py"],
        "icon": "",
    },
    {
        "extensions": ["js"],
        "icon": "",
    },
    {
        "extensions": ["html"],
        "icon": "",
    },
    {
        "extensions": ["css"],
        "icon": "",
    },
    {
        "extensions": ["go"],
        "icon": "",
    },
    {
        "extensions": ["java"],
        "icon": "",
    },
    {
        "extensions": ["c", "h"],
        "icon": "",
    },
    {
        "extensions": ["cpp"],
        "icon": "",
    },
    {
        "extensions": ["rb"],
        "icon": "",
    },
    {
        "extensions": ["php"],
        "icon": "",
    },
    {
        "extensions": ["swift"],
        "icon": "",
    },
    {
        "extensions": ["dart"],
        "icon": "",
    },
    {
        "extensions": ["scala"],
        "icon": "",
    },
    {
        "extensions": ["lua"],
        "icon": "",
    },
    {
        "extensions": ["ts", "tsx"],
        "icon": "",
    },
    {
        "extensions": ["bash", "sh", "zsh", "fish"],
        "icon": "",
    },
    {
        "extensions": ["cs"],
        "icon": "",
    },
    {
        "extensions": ["hs"],
        "icon": "",
    },
    {
        "extensions": ["clj"],
        "icon": "",
    },
    {
        "extensions": ["ex"],
        "icon": "",
    },
    {
        "extensions": ["erl"],
        "icon": "",
    },
    {
        "extensions": ["coffee"],
        "icon": "",
    },
    {
        "extensions": ["jsx", "react"],
        "icon": "",
    },
    {
        "extensions": ["vue"],
        "icon": "",
    },
    {
        "extensions": ["ng"],
        "icon": "",
    },
    {
        "extensions": ["svelte"],
        "icon": "",
    },
    {
        "extensions": ["docker"],
        "icon": "",
    },
    {
        "extensions": ["tf"],
        "icon": "",
    },
    {
        "extensions": ["json"],
        "icon": "",
    },
    {
        "extensions": ["xml"],
        "icon": "󰗀",
    },
    {
        "extensions": ["md"],
        "icon": "",
    },
    {
        "extensions": ["ini", "cfg", "toml", "bat", "cmd", "vbs", "vba", "reg", "yml", "yaml", "log"],
        "icon": "",
    },
    {
        "extensions": ["sql", "sqlite", "mysql", "psql", "mongo", "redis", "cassandra", "hbase", "oracle", "db2", "sybase", "informix", "teradata", "netezza", "greenplum", "vertica", "redshift", "snowflake", "bigquery"],
        "icon": "",
    },
    {
        "extensions": ["lock"],
        "icon": "",
    },
    {
        "extensions": ["exe"],
        "icon": ""
    },
    {
        "extensions": ["lnk"],
        "icon": ""
    }
]

func get_icon_data(extension: String) -> String:
    for icon in ICONS:
        if extension in icon["extensions"]:
            return icon["icon"]
    return " "
