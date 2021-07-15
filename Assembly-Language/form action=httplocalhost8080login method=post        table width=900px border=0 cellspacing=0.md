```html
<form action="http://localhost:8080/login" method="post">
        <table width="900px" border="0" cellspacing="0">
            <tr>
                <td style="padding:30px">
                    <div style="height:470px">
                        <b>&nbsp;&nbsp;首页&nbsp;&raquo;&nbsp;个人用户登录</b>
                        <div>
                            <table width="85%" border="0" cellspacing="0">
                                <tr>
                                    <td>
                                        <div id="logindiv">
                                            <table width="100%" border="0" cellspacing="0">
                                                <tr>
                                                    <td style="text-align:center; padding-top:20px">
                                                        <img src="${pageContext.request.contextPath }images/logintitle.gif"
                                                             width="150" height="30"/>
                                                    </td>
                                                </tr>
                                                <tr>
                                                    <td style="text-align:center;padding-top:20px;"><font
                                                            color="#ff0000">${requestScope["register_message"]}</font>
                                                    </td>
                                                </tr>
                                                <tr>
                                                    <td style="text-align:center">
                                                        <table width="80%" border="0" cellspacing="0"
                                                               style="margin-top:15px ;margin-left:auto; margin-right:auto">
                                                            <tr>
                                                                <td
                                                                        style="text-align:right; padding-top:5px; width:25%">
                                                                    用户名：
                                                                </td>
                                                                <td style="text-align:left"><input name="username"
                                                                                                   type="text"
                                                                                                   class="textinput"/>
                                                                </td>
                                                            </tr>
                                                            <tr>
                                                                <td style="text-align:right; padding-top:5px">密&nbsp;&nbsp;&nbsp;&nbsp;码：</td>
                                                                <td style="text-align:left"><input name="password"
                                                                                                   type="password"
                                                                                                   class="textinput"/>
                                                                </td>
                                                            </tr>
                                                            <tr>
                                                                <td colspan="2" style="text-align:center">
                                                                    <input type="checkbox" name="checkbox"
                                                                           value="checkbox01"/>记住用户名
                                                                    &nbsp;&nbsp;
                                                                    <input type="checkbox" name="checkbox"
                                                                           value="checkbox02"/> 自动登录
                                                                </td>
                                                            </tr>
                                                            <tr>
                                                                <td colspan="2"
                                                                    style="padding-top:10px; text-align:center">
                                                                    <input name="image" type="image"
                                                                           onclick="return formcheck()"
                                                                           src="${pageContext.request.contextPath }images/loginbutton.gif"
                                                                           width="90" height="30"/>
                                                                </td>
                                                            </tr>

                                                            <tr>
                                                                <td colspan="2" style="padding-top:10px">
                                                                    <img src="${pageContext.request.contextPath }images/loginline.gif"
                                                                         width="241" height="10"/>
                                                                </td>
                                                            </tr>
                                                            <!-- <tr>
                                                                <td colspan="2"
                                                                    style="padding-top:10px; text-align:center"><a
                                                                    href="register.jsp"><img name="image"
                                                                        src="images/signupbutton.gif" width="135" height="33" />
                                                                </a></td>
                                                            </tr> -->
                                                        </table>
                                                    </td>
                                                </tr>
                                            </table>
                                        </div>
                                    </td>

                                </tr>
                            </table>
                        </div>
                    </div>
                </td>
            </tr>
        </table>
    </form>
```

