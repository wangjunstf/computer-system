# Git 简明教程

本教程适合初学者查阅，不适合新手首次学习。

## 一、本地仓库

1. 进入项目目录

2. 版本库初始化

   ```shell
   $ git init
   Initialized empty Git repository in /home/mygit/test/learn-git/.git/
   ```

3. 创建，查看和回退历史版本库

   ```shell
   # 添加文件 可以一个或多个文件 此时文件进入 暂存区
   $ git add readme.txt  
   
   # 此时刚添加的文件正式加入版本库，后续可恢复到提前前的状态
   $ git commit -m "add readme.txt"     
   [master (root-commit) a1f54c2] add readme.txt
    1 file changed, 1 insertion(+)
    create mode 100644 readme.txt
   
   # 添加新文件
   $ git commit -m "add file"            
   [master 3a3853a] add file
    1 file changed, 1 insertion(+)
    create mode 100644 file
    
   # 添加修改过的文件 readme.txt 到暂存区
   $ git add readme.txt                  
   
   # 把刚添加的文件放进版本库
   $ git commit -m "add This project help you to learn git."   
   [master bd8b50d] add This project help you to learn git.
    1 file changed, 2 insertions(+)
   
   # 查看版本库信息
   $ git log                             
   commit bd8b50d5b5062ac342f82f249bffba7d6baf0036 (HEAD -> master)
   Author: wangjunstf <2634683912@qq.com>
   Date:   Fri Apr 23 10:05:09 2021 +0800
       add This project help you to learn git.
   
   commit 3a3853a5374156ea2f7fdb7e07f06077636bd4bb
   Author: wangjunstf <2634683912@qq.com>
   Date:   Fri Apr 23 09:54:23 2021 +0800
       add file
   ......
   
   # 简化版本库信息输出
   $ git log --pretty=oneline            
   bd8b50d5b5062ac342f82f249bffba7d6baf0036 (HEAD -> master) add This project help you to learn git.
   3a3853a5374156ea2f7fdb7e07f06077636bd4bb add file
   a1f54c2045fca053248ee67fffff087a478dfa72 add readme.txt
   ```

4. 版本回退

   ```shell
   # 回退到上一个版本
   $ git reset --hard HEAD^                
   HEAD is now at 3a3853a add file
   
   # 回退到指定版本,本指令和上一条指令的执行效果相同
   $ git reset --hard 3a3853               
   HEAD is now at 3a3853a add file
   
   # 查看历史记录  
   $ git reflog														
   3a3853a (HEAD -> master) HEAD@{0}: reset: moving to 3a3853
   3a3853a (HEAD -> master) HEAD@{1}: reset: moving to HEAD^
   bd8b50d HEAD@{2}: commit: add This project help you to learn git.
   3a3853a (HEAD -> master) HEAD@{3}: commit: add file
   a1f54c2 HEAD@{4}: commit (initial): add readme.txt
   #如果想恢复到调用reset之前的版本，可以用该命令查看指定的版本号恢复
   ```

5. 查看和修改工作区内容

   ```shell
   # 查看工作区信息
   $ git status                            
   On branch master
   nothing to commit, working tree clean
   
   # 修改完文件之后，该命令能显示工作区的当前状态
   $ git status														
   On branch master
   Changes not staged for commit:
     (use "git add <file>..." to update what will be committed)
     (use "git checkout -- <file>..." to discard changes in working directory)
   	modified:   readme.txt
   no changes added to commit (use "git add" and/or "git commit -a") 
   
                   												# modified:   readme.txt 表示readme.txt已经修改过，但还没放入暂存区
   $ git add readme.txt                    # 将readme.txt 加入 暂存区
   
   $ git status
   On branch master
   Changes to be committed:
     (use "git reset HEAD <file>..." to unstage)
   
   	modified:   readme.txt
   # modified:   readme.txt 表示readme.txt已经添加到暂存区，还未加入版本库
   
   
   # 查看工作区和版本库里面最新版本的区别  HEAD^ 次新版本
   $ git diff HEAD -- readme.txt           
   diff --git a/readme.txt b/readme.txt
   index d344129..2200906 100644
   --- a/readme.txt
   +++ b/readme.txt
   @@ -1 +1,3 @@
    This is a readme file.
   +
   +This is a guide.
   
   # 丢弃工作区的修改 ,回到最近提交到版本库的版本，暂存区不变
   $ git checkout -- readme.txt						
   
   # 把暂存区的内容放回工作区
   $ git reset HEAD file                   
   
   # 丢弃工作区的修改，上面和本条命令结合，撤销了提交到暂存区的修改
   $ git checkout -- file                  
   ```

6. 删除文件

   ```shell
   #删除工作区文件
   $ git rm test.txt 
   
   #将删除操作提交到版本库
   $ git commit -m "remove test.txt"        
   ```

## 二、远程仓库提交(GitHub)

1. 创建远程仓库

   在https://github.com/网站上创建仓库

2. 将远程仓库克隆到本地

   ```shell
   $ git clone git@github.com:wangjunstf/learn-git.git
   ```

3. 在远程仓库添加ssh公钥，具体百度

4. 与远程仓库建立连接

   ```shell
   $ git remote add learngit git@github.com:wangjunstf/learn-git.git
   ```

5. 将本地库的所有内容推送到远程库

   ```shell
   $ git push -u learngit main
   ```

6. 查看远程仓库

   ```shell
   $ git remote -v
   learngit	git@github.com:wangjunstf/learn-git.git (fetch)
   learngit	git@github.com:wangjunstf/learn-git.git (push)
   origin	git@github.com:wangjunstf/learn-git.git (fetch)
   origin	git@github.com:wangjunstf/learn-git.git (push)
   ```



## 三、与远程仓库同步

1. 抓取远程仓库更新内容

   ```shell
   $ git fetch learngit main
   From github.com:wangjunstf/learn-git
    * branch            main       -> FETCH_HEAD
      b291452..4bc596d  main       -> learngit/main
   ```

2. 比较远程更新和本地版本库的差异

   ```shell
   $ git log main.. learngit/main
   commit 4bc596dc726ee376e1b8c4b4264c35dfe80e0143 (origin/main, origin/HEAD, learngit/main)
   Author: jun <47825821+wangjunstf@users.noreply.github.com>
   Date:   Fri Apr 23 15:42:13 2021 +0800
   
       Update hello.txt
   
   commit 3535e258a9ed18139bcca5a11b0ccd0624f51351
   Author: jun <47825821+wangjunstf@users.noreply.github.com>
   Date:   Fri Apr 23 15:41:37 2021 +0800
   
       Update hello.txt
   ```

   可以看到，远程比本地多了两次commit

3. 合并远程更新到本地

   ```shell
   $ git merge learngit/main
   Updating b291452..4bc596d
   Fast-forward
    hello.txt | 2 ++
    1 file changed, 2 insertions(+)
   ```



## 四、分支管理

### 4.1 分支创建与合并

1. 创建于切换分支

   ```shell
   $ git branch dev      		# 创建分支 dev
   $ git checkout dev    		# 切换到分支dev
   $ git checkout -b dev 	  # 创建并切换到分支dev  相当于以上两条命令
   ```

2. 显示分支情况

   ```shell
   $ git branch
   * dev
     main
   ```

3. 修改某些文件并提交到dev

   ```shell
   $ git commit -m "branch test"
   [dev 0289255] branch test
    1 file changed, 2 insertions(+)
   ```

4. 切换回主分支

   ```shell
   $ git checkout main
   Switched to branch 'main'
   ```

5. 与合并dev分支到main分支

   ```shell
   $ git merge dev
   Updating 4bc596d..0289255
   Fast-forward
    README.md | 2 ++
    1 file changed, 2 insertions(+)
   ```

6. 合并完成，删除dev分支

   ```shell
   $ git branch -d dev
   Deleted branch dev (was 0289255).
   ```

### 4.2 分支冲突解决

1. 创建分支feature，并在文件结尾添加"Creating a new branch is quick AND simple."

   ```shell
   $ git commit -m "feature"
   $ git add README.md 
   $ git commit -m "And simeple"
   [feature 01d4edf] feature
    1 file changed, 1 insertion(+), 1 deletion(-)
   ```

2. 切回主分支并修改之前添加的内容，将AND simple 改为& simple，并提交

   ```shell
   $ git checkout main
   git add README.md 
   git commit -m "& simple"
   [main 329cbc2] & simple
    1 file changed, 1 insertion(+), 1 deletion(-)
   ```

3. 合并

   ```shell
   $ git merge feature
   Auto-merging README.md
   CONFLICT (content): Merge conflict in README.md
   Automatic merge failed; fix conflicts and then commit the result.
   ```

4. 查看存在冲突的文件

   ```shell
   $ git status
   On branch main
   You have unmerged paths.
     (fix conflicts and run "git commit")
     (use "git merge --abort" to abort the merge)
   
   Unmerged paths:
     (use "git add <file>..." to mark resolution)
     
   	both modified:   README.md
   ```

   

5. 冲突解决。因为两个分支修改了同样的内容，git无法自动合并，需要手动合并。

   ```shell
   $ vim README.md          				# 打开存在冲突的文件
   git会把存在冲突的部分标记为以下形式，我们需要手动修改它们
   <<<<<<< HEAD
   Creating a new branch is quick & simple.
   =======
   Creating a new branch is quick AND simple.
   >>>>>>> feature1
   
   将以上内容改为：
   Creating a new branch is quick & simple.
   
   $ git add README.md
   $ git commit -m "conflict fixed"
   [main ae5e8c4] conflict fixed   # 表示冲突已经解决  
   
   $ git branch -d feature         # 删除已经合并的分支
   ```

6. 查看分支合并图

   ```shell
   $ git log --graph
   ```

## 五、分支管理策略

### 5.1 --no-ff模式

git默认采用Fast forward模式，Fast forward会丢失合并前的信息。--no-ff模式合并，即禁用Fast forward

```shell
$ git merge --no-ff -m "merge with no-ff" dev # 此时合并前会为提交之前的版本 提交一个commit
```



### 5.2 bug分支

1. 隐藏工作现场，此时一般是有紧急任务，比如修复某分支bug，但当前分支的修改还没提交到版本库。

   ```shell
   $ git stash					# 保存工作现场，一会恢复，工作区和暂存区的内容都会恢复
   ```

2. 查看工作现场

   ```shell
   $ git stash list
   stash@{0}: WIP on dev: ae5e8c4 conflict fixed
   ```

3. 修复完其他分支的bug，回到之前的分支

   ```shell
   $ git stash apply    # 恢复到之前的工作现场   stash内容并不删除
   $ git stash drop     # 删除最近一次保存的stash
   
   $ git stash pop      #相当于执行以上两条命令，在恢复现场的同时删除stash
   $ git stash apply stash@{0}  # 恢复到指定stash
   ```

4. 合并其他分支的commit（bug修复）到当前分支

   ```shell
   $ git cherry-pick 4c805e2
   ```

5. 强制删除分支（未合并分支）

   ```shell
   $ git branch -D feature-vulcan
   ```

   

## 六、多人协作

假如现有一同事在同时维护一个仓库。

### 6.1 提交远程时的冲突

1. 默认克隆的仓库只能看到main分支，需要使用其他分支，需要创建分支并关联。

   ```shell
   $ git checkout -b dev origin/dev
   ```

2. 指定本地dev分支与远程dev分支建立连接

   ```shell
   $ git branch --set-upstream-to=origin/dev dev
   ```

3. ```shell
   $ git pull       # 作用为与远程合并，这时冲突依然存在，需要打开存在冲突的文件，手动修改冲突的部分，在commit提交，就可提交远程仓库
   ```

4. 与远程分支建立映射

   ```shell
   $ git branch -u origin/dev dev
   Branch 'dev' set up to track remote branch 'dev' from 'origin'.
   ```

   



### 6.2 与远程同步时的冲突

出现问题的原因大多是远程仓库，版本高于本地，即要提交的文件，远程仓库已经有了修改过的版本，这时需要手动解决冲突。

1. 将远程仓库的所有更改取回本地

   ```shell
   $ git fetch origin
   ```

2. 比较远程仓库和本地仓库的差异

   ```shell
   $ git log dev..origin/dev     # 查看远程仓库的最新commit信息
   ```

3. 统计文件的改动

   ```shell
   $ git diff --stat dev origin/dev
   ```

4. 与远程分支合并

   ```shell
   $ git merge origin/dev
   ```

5. 一键同步远程分支

   ```shell
   $ git pull        # 相当于 git fetch origin + git merge origin/dev
   ```

6. 若存在冲突，则打开冲突的文件，修改冲突的部分，假设mac.txt冲突

   ```shell
   $ vim mac.txt            # 修改冲突的部分
   $ git add mac.txt
   $ git commit -m "fix conflict"
   $ git push origin dev    #这样就可以提交了
   ```

7. 将多个提交合并为一个提交

   ```shell
   # git rebase -i  [startpoint]  [endpoint]
   $ git rebase -i 36224db     # git log查看
   $ git rebase -i HEAD~3      # 和上条命令等小
   
   # 打开后，将pick改成特定的字母即可
   # 例如 
    pick 7885574 add second
    pick cdcd83f add second
    pick 4db5372 add 1.txt
   # 将以上内容改为
    pick 7885574 add second
    s cdcd83f add second
    s 4db5372 add 1.txt
   # 保存退出后，弹出的界面修改注释,保存就可将三条提交合并为一条
   ```



## 七、标签

1. 给当前版本打上标签

   ```shell
   $ git tag v0.9
   $ git tag v0.9 4db537274   # 查询commit id
   
   $ git tag -a v0.1 -m "version 0.1 released" 4db537274  # 创建待说明的标签
   ```

2. 查看标签

   ```shell
   $ git tag
   ```

3. 查看标签信息

   ```shell
   $ git show v0.9
   ```

4. 标签删除

   ```shell
   $ git tag -d v0.9
   ```

5. 推送某个版本到远程

   ```shell
   git push origin v1.0
   ```

6. 把本地尚未推送的标签推送到远程

   ```
   git push origin --tags
   ```

7. 删除远程标签

   ```shell
   $ git tag -d v0.9
   $ git push origin :refs/tags/v0.9
   ```

   