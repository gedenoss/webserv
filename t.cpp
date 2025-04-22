void Request::checkRawRequest(const std::string &rawRequest, Config &config, std::istringstream &stream, std::string line ){
	if (rawRequest.size() >	8000) {
		_errorCode = 431;	
		return;
	}
	if (!std::getline(stream, line)	|| line.empty()) {
		_errorCode = 400;	
		return;
	}
	if (!line.empty() && line[line.size() -	1] == '\r')	{
		line.erase(line.size() - 1);
	}
	std::istringstream requestLine(line);
	std::string	method,	url, httpVersion;
	requestLine	>> method >> url >> httpVersion;
	if (url.length() > 8000) {
		_errorCode = 414;	
		return NULL;
	}
	setMethod(method);
	setUrl(url);
	setHttpVersion(httpVersion);
	

	if (!isValidMethod())
	{
		_errorCode = 400;	
		return NULL;
	}
	else if(method != "GET"	&& method != "POST"	&& method != "DELETE"){
			_errorCode = 501;
			return;
	}

	if (!isValidUrl()) {
				_errorCode = 400;	
		return;
	}


	if (!isMethodAllowedForRoute(config)) {
		_errorCode = 405;	
		return;
	}

	
	if (!isValidHttpVersion()) {
		_errorCode = 505;	
		return;
	}
}