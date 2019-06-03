metadata {
	definition (name: "ESP8266", namespace: "jrendean", author: "jrendean") {
		capability "Refresh"
        capability "Sensor"
        capability "Contact Sensor"
		capability "Polling"
	}

    tiles(scale:2) {
        multiAttributeTile(name:"rich-control", type: "switch"){
            tileAttribute ("device.contact", key: "PRIMARY_CONTROL") {
                 attributeState "open", label:'${name}', icon:"st.contact.contact.open", backgroundColor:"#ffa81e"
                 attributeState "closed", label:'${name}', icon:"st.contact.contact.closed", backgroundColor:"#79b821"
 			}
            tileAttribute ("currentIP", key: "SECONDARY_CONTROL") {
             	 attributeState "currentIP", label: 'fdg'
 			}
        }

        standardTile("refresh", "device.contact", width: 2, height: 2, inactiveLabel: false,  decoration: "flat") {
            state("default", label:"", action:"refresh.refresh", icon:"st.secondary.refresh")
        }
        
        main("rich-control")
        details(["rich-control", "contact", "refresh"])
    }
    
    simulator {
    	
    }
    
    preferences {
		input(name: "ip", type: "text", title: "IP Address", required: true, displayDuringSetup: true)
		input(name: "port", type: "number", title: "Port", defaultValue: 8080, required: true, displayDuringSetup: true)
        input(name: "esp8266Type", type: "enum", title: "Device Type", options:["Contact", "Switch", "Temp"], required: true, displayDuringSetup: true)
	}
}

def updated() {
	log.debug("Updated with settings: $settings")
	state.dni = ""
	updateDNI()
	updateSettings()
}

private def updateDNI() {
	log.debug("updateDNI")
	if (!state.dni || state.dni != device.deviceNetworkId || (state.mac && state.mac != device.deviceNetworkId)) {
        device.deviceNetworkId = createNetworkId(settings.ip, settings.port)
		state.dni = device.deviceNetworkId
	}
}

private String createNetworkId(ipaddr, port) {
	if (state.mac) {
		return state.mac
	}
	def hexIp = ipaddr.tokenize('.').collect {
		String.format('%02X', it.toInteger())
	}.join()
	def hexPort = String.format('%04X', port.toInteger())
	return "${hexIp}:${hexPort}"
}

def updateSettings() {
	def headers = [:] 
	headers.put("HOST", getHostAddress())
	headers.put("Content-Type", "application/x-www-form-urlencoded")
    
	return new physicalgraph.device.HubAction(
		method: "POST",
		path: "/updateSettings",
		body: "callbackAddress=${device.hub.getDataValue("localIP")}&callbackPort=${device.hub.getDataValue("localSrvPortTCP")}&name=${device.name}",
		headers: headers
	)
}

def parse(String description) {
	log.debug "Executing 'parse'"
    log.debug "Parsing '${description}'"

	def msg = parseLanMessage(description)
	log.debug "parseLanMessage: ${msg}"

    if (!state.mac || state.mac != msg.mac) {
		log.debug "Setting deviceNetworkId to MAC address ${msg.mac}"
		state.mac = msg.mac
	}
 
	def result = []
    def value = ""
    def bodyString = msg.body
    
	if (bodyString) {
        def json = msg.json
        if (json?.name == "contact") {
        	value = json.status == "1" ? "closed" : "open"
            log.debug "contact status ${value}"
			result << createEvent(name: "contact", value: value)
        } 
	}

	result
}


def refresh() {
    log.debug "Executing 'refresh'"
    
    updateDNI()
    
	def results = new physicalgraph.device.HubAction(
    	method: "GET",
    	path: "/getstatus",
    	headers: [
        	HOST: "${getHostAddress()}"
    	])
        
    results
}

def poll() {
    log.debug "Executing 'poll'"

    def results = new physicalgraph.device.HubAction(
    	method: "GET",
    	path: "/getstatus",
    	headers: [
        	HOST: "${getHostAddress()}"
    	])
}

private getTime() {
	// This is essentially System.currentTimeMillis()/1000, but System is disallowed by the sandbox.
	((new GregorianCalendar().time.time / 1000l).toInteger()).toString()
}

private getCallBackAddress() {
	device.hub.getDataValue("localIP") + ":" + device.hub.getDataValue("localSrvPortTCP")
}

private getHostAddress() {
	def ip = settings.ip
	def port = settings.port

	//log.debug "Using ip: ${ip} and port: ${port} for device: ${device.id}"
	return ip + ":" + port
}
/*
private getHostAddress() {
	def ip = getDataValue("ip")
	def port = getDataValue("port")

	if (!ip || !port) {
		def parts = device.deviceNetworkId.split(":")
		if (parts.length == 2) {
			ip = parts[0]
			port = parts[1]
		} else {
			log.warn "Can't figure out ip and port for device: ${device.id}"
		}
	}
	log.debug "Using ip: ${ip} (${convertHexToIP(ip)}) and port: ${port} (${convertHexToInt(port)}) for device: ${device.id}"
    return convertHexToIP(ip) + ":" + convertHexToInt(port)
}
*/
private Integer convertHexToInt(hex) {
 	Integer.parseInt(hex,16)
}

private String convertHexToIP(hex) {
 	[convertHexToInt(hex[0..1]),convertHexToInt(hex[2..3]),convertHexToInt(hex[4..5]),convertHexToInt(hex[6..7])].join(".")
}