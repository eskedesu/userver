from chaotic_openapi.front import model

from chaotic.front import types


def test_empty_swagger(simple_parser):
    assert simple_parser({
        'swagger': '2.0',
        'info': {},
        'paths': {},
    }) == model.Service(name='test', description='', operations=[])


def test_swagger_body_schema(simple_parser):
    assert simple_parser(
        {
            'swagger': '2.0',
            'info': {},
            'paths': {
                '/': {
                    'get': {
                        'parameters': [
                            {
                                'in': 'body',
                                'required': True,
                                'schema': {
                                    'type': 'boolean',
                                },
                            },
                        ],
                        'responses': {},
                    },
                },
            },
        },
    ) == model.Service(
        name='test',
        description='',
        operations=[
            model.Operation(
                description='',
                path='/',
                method='get',
                operationId='Get',
                parameters=[],
                requestBody=[
                    model.RequestBody(
                        content_type='application/json',
                        schema=types.Boolean(),
                        required=True,
                    )
                ],
                responses={},
                security=[],
            )
        ],
    )


def test_swagger_securuty(simple_parser):
    assert simple_parser(
        {
            'swagger': '2.0',
            'info': {},
            'securityDefinitions': {
                'api_key': {
                    'type': 'apiKey',
                    'name': 'api_key',
                    'in': 'header',
                },
                'oauth_implicit': {
                    'type': 'oauth2',
                    'flow': 'implicit',
                    'authorizationUrl': 'https://example.com/api/oauth/dialog',
                    'scopes': {'read': 'read data', 'other': '-'},
                },
                'oauth_code': {
                    'type': 'oauth2',
                    'flow': 'accessCode',
                    'authorizationUrl': 'https://example.com/api/oauth/dialog',
                    'tokenUrl': 'https://example.com/api/oauth/token',
                    'scopes': {'write': 'modify data', 'read': 'read data', 'other': '-'},
                },
            },
            'security': {
                'api_key': [],
                'oauth_implicit': ['read'],
                'oauth_code': ['read', 'write'],
            },
            'paths': {
                '/': {
                    'get': {
                        'parameters': [],
                        'responses': {},
                        'security': {
                            'api_key': [],
                            'oauth_implicit': ['read'],
                            'oauth_code': ['read'],
                        },
                    },
                    'post': {
                        'parameters': [],
                        'responses': {},
                        'security': {
                            'api_key': [],
                            'oauth_implicit': ['write'],
                            'oauth_code': ['write'],
                        },
                    },
                    'put': {'parameters': [], 'responses': {}},
                }
            },
        },
    ) == model.Service(
        name='test',
        description='',
        security={
            '<inline>#/securityDefinitions/api_key': model.ApiKeySecurity(
                description='', name='api_key', in_=model.SecurityIn.header
            ),
            '<inline>#/securityDefinitions/oauth_implicit': model.OAuthSecurity(
                description='',
                flows=[
                    model.ImplicitFlow(
                        refreshUrl='',
                        scopes={'read': 'read data', 'other': '-'},
                        authorizationUrl='https://example.com/api/oauth/dialog',
                    ),
                ],
            ),
            '<inline>#/securityDefinitions/oauth_code': model.OAuthSecurity(
                description='',
                flows=[
                    model.AuthCodeFlow(
                        refreshUrl='',
                        scopes={'write': 'modify data', 'read': 'read data', 'other': '-'},
                        authorizationUrl='https://example.com/api/oauth/dialog',
                        tokenUrl='https://example.com/api/oauth/token',
                    ),
                ],
            ),
        },
        operations=[
            model.Operation(
                description='',
                path='/',
                method='get',
                operationId='Get',
                parameters=[],
                responses={},
                requestBody=[],
                security=[
                    model.ApiKeySecurity(description='', name='api_key', in_=model.SecurityIn.header),
                    model.OAuthSecurity(
                        description='',
                        flows=[
                            model.ImplicitFlow(
                                refreshUrl='',
                                scopes={'read': 'read data'},
                                authorizationUrl='https://example.com/api/oauth/dialog',
                            ),
                        ],
                    ),
                    model.OAuthSecurity(
                        description='',
                        flows=[
                            model.AuthCodeFlow(
                                refreshUrl='',
                                scopes={'read': 'read data'},
                                authorizationUrl='https://example.com/api/oauth/dialog',
                                tokenUrl='https://example.com/api/oauth/token',
                            ),
                        ],
                    ),
                ],
            ),
            model.Operation(
                description='',
                path='/',
                method='post',
                operationId='Post',
                parameters=[],
                responses={},
                requestBody=[],
                security=[
                    model.ApiKeySecurity(description='', name='api_key', in_=model.SecurityIn.header),
                    model.OAuthSecurity(
                        description='',
                        flows=[
                            model.ImplicitFlow(
                                refreshUrl='',
                                scopes={},
                                authorizationUrl='https://example.com/api/oauth/dialog',
                            ),
                        ],
                    ),
                    model.OAuthSecurity(
                        description='',
                        flows=[
                            model.AuthCodeFlow(
                                refreshUrl='',
                                scopes={'write': 'modify data'},
                                authorizationUrl='https://example.com/api/oauth/dialog',
                                tokenUrl='https://example.com/api/oauth/token',
                            ),
                        ],
                    ),
                ],
            ),
            model.Operation(
                description='',
                path='/',
                method='put',
                operationId='Put',
                parameters=[],
                responses={},
                requestBody=[],
                security=[
                    model.ApiKeySecurity(description='', name='api_key', in_=model.SecurityIn.header),
                    model.OAuthSecurity(
                        description='',
                        flows=[
                            model.ImplicitFlow(
                                refreshUrl='',
                                scopes={'read': 'read data'},
                                authorizationUrl='https://example.com/api/oauth/dialog',
                            )
                        ],
                    ),
                    model.OAuthSecurity(
                        description='',
                        flows=[
                            model.AuthCodeFlow(
                                refreshUrl='',
                                scopes={'write': 'modify data', 'read': 'read data'},
                                authorizationUrl='https://example.com/api/oauth/dialog',
                                tokenUrl='https://example.com/api/oauth/token',
                            )
                        ],
                    ),
                ],
            ),
        ],
    )
