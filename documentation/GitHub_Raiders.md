基于GitHub参与RealBoard4088开源项目指南
--------------------------------------------------
 GitHub是一个非常棒的项目托管网站，利用Git极其强大的克隆和分支功能，让我们可以非常方便的参与别人的开源项目，当然也可以让别人参与我们的开源项目。下面就让我带着大家一起来畅游GitHub知识的海洋吧！


----------


###一、注册GitHub账号###
登陆[GitHub主页][1]，注册GitHub账号
###二、安装Git###
msygit是windows版的Git，从[msysgit][2]下载，然后按默认的选项安装即可，安装成功后可以在开始菜单里面找到“Git”->“Git Bash“


###三、配置Git###
第一步创建ssh key，开始菜单里面点击“Git”->“Git Bash”蹦出一个类似于命令行窗口的东西，输入  

    $ ssh-keygen -t rsa -C "youremail@example.com"  

你需要将邮件地址换为你自己的地址，然后一路回车,按默认安装即可


第二步登陆[GitHub][3]，打开“Account settings”就是主页右上方小扳手的图  标，“SSH Keys”页面：然后，点“Add SSH Key”，填上任意Title，在Key文本框里粘贴id_rsa.pub（id_rsa.pub文件在C/Users/example-name/.ssh目录下）文件的内容：
![此处输入图片的描述][4]


点“Add Key”，你就应该看到已经添加的Key：
![此处输入图片的描述][5]


为什么GitHub需要SSH Key呢？因为GitHub需要识别出你推送的提交确实是你推送的，而不是别人冒充的，而Git支持SSH协议，所以，GitHub只要知道了你的公钥，就可以确认只有你自己才能推送。
当然，GitHub允许你添加多个Key。假定你有若干电脑，你一会儿在公司提交，一会儿在家里提交，只要把每台电脑的Key都添加到GitHub，就可以在每台电脑上往GitHub推送了。

第三步因为Git是分布式版本控制系统，所以每个机器都必须自报家门：你的名字和Email地址。最后一步设置便是自报家门，在命令行环境下输入

    $ git config --global user.name "your name"
    $ git config --global user.email "youremail@example.com"

###四、克隆RealBoard4088本地仓库###

第一步访问[RealBaoad4088主页](https://github.com/RT-Thread/RealBoard4088)，点击右上角的“fork”，就在自己的账号下克隆了一个RealBoard4088远程仓库


第二步登陆自己的主页`https://github.com/user_name/RealBoard4088`找到自己仓库的Git地址（在主页的右下方“SSH clone URL”框里）复制下来，第三步git clone命令用到的就是这个地址

第三步在命令行环境下，cd到一个你想要放置RealBoard4088工程的路径下输入 

    $ git clone git@github.com:lifancn/RealBoard4088.git
    Cloning into ‘RealBoard4088’...
    Remote: Reusing existing pack: 4242 done.
    Remote: Total 4242 （delta 0），reused 0（delta 0）
    Receiving objects：100% （4242/4242）,36.71 MiB | 16.00 KiB/s,
    Resolving deltas:100%(2406/2406),done.
    Checking connectivity...done.
    Checking out files:100%(4030/4030),done.
###五、本地仓库Git使用###
这里我示范在RealBoard4088目录下里添加一个READTEST.md文件


----------


第一步在RealBoard4088目录下新建一个READTEST.md文件，在文件中写入“###This is a test File###”保存

第二步将本次修改内容提交到当前分支 （分3小步）  
①查看git仓库目录下那里有修改

    $ git status 
    #On branch master
    #Untracked files
    #   （use “git add <file>...” to include in what will be committed）
    #		READTEST.md	
    Nothing added to commit but untracked files present(use “git add” to track)

②将文件修改添加到暂存区  

    $ git add READTEST.md 

③把暂存区所有的内容提交到当前分支

    $ git commit -m “Add READTEST.md file”
    [master d8d4f2c] add READTEST.md file
    1 file changed,1 insertion(+)
    Create mode 100644 READTEST.md 
###六、将本地仓库改动推送远程仓库###


     $ git push origin master
     Counting objects：4，done.
     Delta compression using up to 2 threads
     Compressing objects：100%（2/2）,done.
     Writing objects：100%（3/3）,290 bytes | 0 bytes/s,done.
     Total 3 (delata1),reused 0 (delta 0)
    To git@github.com:lifancn/RealBoard4088.git
     Bd9916b..d8d4f2c master->master
  推送成功后，我们立刻就可以在GitHub页面上看到刚才提交的文件内容
![此处输入图片的描述][7]
###七、给主仓库发pull request###
打开`https://github.com/user_name/RealBoard4088`,点击右上角的“pull request”，点击New Pull Request->Create Pull Request会打开创建pull request的界面，如下
![此处输入图片的描述][8]
点击Send pull request就OK了，至于采不采用你的源码，我相信只要你的修改有利于开源社区，肯定会被采纳。

注意：我给出的网址带user_name的都需要替换为你自己的GitHub仓库地址


  [1]: http://www.github.com/
  [2]: http://msysgit.github.io/
  [3]: http://www.github.com/
  [4]: https://raw.github.com/lifancn/DocFile/master/image/1.jpg
  [5]: https://raw.github.com/lifancn/DocFile/master/image/2.jpg
  [6]: https://github.com/user_name/RealBoard4088
  [7]: https://raw.github.com/lifancn/DocFile/master/image/4.jpg
  [8]: https://raw.github.com/lifancn/DocFile/master/image/5.jpg
